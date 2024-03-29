<?php

/**
 * Combined menu callback for tests of consumers and access tokens
 */
function _oauth_common_validate_request_callback($type, $unsigned = NULL) {
  try {
    module_load_include('inc', 'oauth_common');

    list($signed, $consumer, $token) = oauth_common_verify_request();

    if ($consumer == NULL) {
      throw new OAuthException('Missing consumer token');
    }

    if (!$signed && $unsigned != 'unsigned') {
      throw new OAuthException("The request wasn't signed");
    }

    if ($token == NULL && $type == 'access token') {
      throw new OAuthException('Missing access token');
    }
  }
  catch (OAuthException $e) {
    drupal_add_http_header('Status', '401 Unauthorized: ' . $e->getMessage());
    drupal_add_http_header('WWW-Authenticate', sprintf('OAuth realm="%s"', url('', array('absolute' => TRUE))));
  }
  exit;
}

/**
 * Menu callback for when something has been authorized - used in both client and provider flow
 *
 * @param $csid Should contain the id of the consumer when used in the client flow
 */
function oauth_common_page_authorized($csid = NULL) {
  // If we have an oauth_token we're acting as a consumer and just got authorized
  if (!empty($_GET['oauth_token'])) {
    //TODO: Add documentation on how to use the callback url with
    $consumer = $csid ? DrupalOAuthConsumer::loadById($csid, FALSE) : FALSE;
    if ($consumer) {
      $request_token = DrupalOAuthToken::loadByKey($_GET['oauth_token'], $consumer, OAUTH_COMMON_TOKEN_TYPE_REQUEST);
    }
    else {
      // Backwards compatibility with 6.x-3.0-beta3
      $request_token = DrupalOAuthToken::load($_GET['oauth_token'], FALSE);
      $consumer = $request_token ? $request_token->consumer : FALSE;
    }
    if (!empty($request_token)) {
      $client = new DrupalOAuthClient($consumer, $request_token);

      $verifier = isset($_GET['oauth_verifier']) ? $_GET['oauth_verifier'] : NULL;

      $access_token = $client->getAccessToken(NULL, array('verifier' => $verifier));
      if ($access_token) {
        // We recieved a new token - save it
        if (!$access_token->in_database) {
          $access_token->write();
        }
        $request_token->delete();
        module_invoke_all('oauth_common_authorized', $consumer, $access_token, $request_token);
      }
    }
  }
  return t('The application has been authorized');
}

/**
 * Form for granting access to the consumer
 */
function oauth_common_form_authorize() {
  module_load_include('inc', 'oauth_common');
  $req = DrupalOAuthRequest::from_request();
  $context = oauth_common_context_from_request($req);

  if (!$context) {
    drupal_set_message(t("Can't find OAuth context, check the site's settings."), 'error');
    return;
  }

  $token = $req->get_parameter('oauth_token');
  $callback = $req->get_parameter('oauth_callback');
  $token = DrupalOAuthToken::loadByKey($token, FALSE, OAUTH_COMMON_TOKEN_TYPE_REQUEST);

  // Check that we have a valid token
  if (!$token) {
    drupal_set_message(t('Please include a valid OAuth token in your request.'), 'error');
    return;
  }

  $consumer = $token->consumer;

  // Redirect to the right form, or present an error.
  global $user;
  if ($user->uid) {
    // There's some strange bug in the ?destination=... handling
    // This is not exactly beautiful, but it gets the work done
    // TODO: Find out why!
    if (drupal_substr($_SERVER['REQUEST_URI'], 0, 2) == '//') {
      header('Location: ' . drupal_substr($_SERVER['REQUEST_URI'], 1), TRUE, 302);
    }

    if (!(user_access('oauth authorize any consumers') || user_access('oauth authorize consumers in ' . $consumer->context))) {
      drupal_set_message(t('You are not authorized to allow external services access to this system.'), 'error');
      return drupal_access_denied();
    }

    if (!empty($context->authorization_options['automatic_authorization']) && $context->authorization_options['automatic_authorization'] && !empty($consumer->callback_url)) {
      // Authorize the request token
      $token->uid = $user->uid;
      $token->authorized = 1;
      $token->services = $context->authorization_options['default_authorization_levels'];
      $token->write(TRUE);

      // Pick the callback url apart and add the token parameter
      $callback = parse_url($consumer->callback_url);
      $query = array();
      parse_str($callback['query'], $query);
      $query['oauth_token'] = $token->key;
      $callback['query'] = http_build_query($query, 'idx_', '&');

      // Return to the consumer site
      header('Location: ' . _oauth_common_glue_url($callback), TRUE, 302);
      exit;
    }

    $tvars = array(
      '@user' => $user->name,
      '@appname' => $consumer->name,
      '@sitename' => variable_get('site_name', ''),
    );

    $title = !empty($context->title) ? $context->title : 'Authorize @appname';
    drupal_set_title(t($title, $tvars), PASS_THROUGH);

    $form = array();

    $form['token'] = array(
      '#type'  => 'value',
      '#value' => $token,
    );

    $message = !empty($context->authorization_options['message']) ? $context->authorization_options['message'] :
      'The application @appname wants to access @sitename on your behalf, check the permissions that you would like the application to have.';
    $form['message'] = array(
      '#type' => 'item',
      '#markup' => t($message, $tvars),
    );

    $message = !empty($context->authorization_options['warning']) ? $context->authorization_options['warning'] :
      'If you don\'t know what @appname is, or don\'t want to give it access to your content, just click here and we\'ll take you away from this page without granting @appname any access to @sitename.';
    $form['warning'] = array(
      '#type' => 'item',
      '#markup' => l(t($message, $tvars), 'oauth/authorization/deny/' . $token->key),
      '#attributes' => array(
        'class' => array('abort-authorization'),
      ),
    );

    $disable_selection = !empty($context->authorization_options['disable_auth_level_selection']) && !empty($context->authorization_options['default_authorization_levels']) && $context->authorization_options['disable_auth_level_selection'];
    if (!$disable_selection) {
      $authorization_title = !empty($context->authorization_options['authorization_title']) ? $context->authorization_options['authorization_title'] :
        'Permissions';
      $form['authorization'] = array(
        '#type' => 'fieldset',
        '#title' => t($authorization_title, $tvars),
      );

      $form['authorization']['levels'] = array(
        '#tree' => TRUE,
      );
      foreach ($context->authorization_levels as $name => $level) {
        $auth_opt = array(
          '#type' => 'checkbox',
          '#title' => t($level['title'], $tvars),
          '#description' => t($level['description'], $tvars),
        );
        $form['authorization']['levels'][$name] = $auth_opt;
      }
    }
    else {
      $form['authorization']['levels'] = array(
        '#tree' => TRUE,
      );
      foreach ($context->authorization_options['default_authorization_levels'] as $level) {
        $form['authorization']['levels'][$level] = array(
          '#type' => 'value',
          '#value' => $level,
        );
      }
    }

    $deny_title = !empty($context->authorization_options['deny_access_title']) ? $context->authorization_options['deny_access_title'] :
      'Deny access';
    $form['deny'] = array(
      '#type' => 'item',
      '#markup' => l(t($deny_title), 'oauth/authorization/deny/' . $token->key),
      '#attributes' => array(
        'class' => array('deny-access'),
      ),
    );

    $grant_title = !empty($context->authorization_options['grant_access_title']) ? $context->authorization_options['grant_access_title'] :
      'Grant access';
    $form['actions'] = array('#type' => 'actions');
    $form['actions']['confirm'] = array(
      '#type'   => 'submit',
      '#value'  => t($grant_title),
    );

    return $form;
  }
  else {
    $query = $_GET;
    unset($query['q']); // why are there so few q's?
    drupal_goto('user/login', array('query' => array(
      'destination' => url('oauth/authorize', array(
        'query' => $query,
      )),
    )));
  }
}

/**
 * Validation of the form for granting access to the consumer
 */
function oauth_common_form_authorize_validate($form, &$form_state) {
  $values = $form_state['values'];
  $got_permission = FALSE;

  $consumer = $values['token']->consumer;
  $context = oauth_common_context_load($consumer->context);

  if (!$context) {
    form_set_error('confirm', t("Can't find OAuth context."));
    return;
  }

  if (!$context->authorization_options['disable_auth_level_selection']) {
    foreach ($context->authorization_levels as $name => $level) {
      if ($values['levels'][$name]) {
        $got_permission = TRUE;
        break;
      }
    }

    if (!$got_permission) {
      form_set_error('confirm', t("You haven't given the application access to anything. Click on 'Deny access' or just close this window if you don't want to authorize it."));
    }
  }
}

/**
 * Form submit handler that grants access to the consumer
 */
function oauth_common_form_authorize_submit(&$form, &$form_state) {
  global $user;
  $values = $form_state['values'];

  // Save the list of all services that the user allowed the
  // consumer to do
  $token = $values['token'];
  $token->uid = $user->uid;
  $token->authorized = 1;
  $consumer = $token->consumer;
  $context = oauth_common_context_load($consumer->context);

  if (!$context) {
    drupal_set_message(t("Can't find OAuth context, check the site's settings."), 'error');
    return;
  }

  // Add services
  if (!empty($values['full_access'])) { // TODO: Full access should be a configurable auth level
    $token->services = array('*');
  }
  elseif (!empty($values['levels'])) {
    $token->services = array_keys(array_filter($values['levels']));
  }
  else {
    $token->services = array();
  }

  $token->write(TRUE);

  if (!empty($consumer->callback_url) && $consumer->callback_url !== 'oob') {
    // Pick the callback url apart and add the token parameter
    $callback = parse_url($consumer->callback_url);
    $query = array();
    if (!empty($callback['query'])) {
      parse_str($callback['query'], $query);
    }
    $query['oauth_token'] = $token->key;
    $callback['query'] = http_build_query($query, 'idx_', '&');

    // Return to the consumer site
    header('Location: ' . _oauth_common_glue_url($callback), TRUE, 302);
    exit;
  }
  else {
    drupal_goto('oauth/authorized');
  }
}

/**
 * Constructs the url to which to return someone who has asked for access to a consumer
 */
function _oauth_common_glue_url($parsed) {
  $uri = isset($parsed['scheme']) ? $parsed['scheme'] . '://' : '';
  $uri .= isset($parsed['user']) ? $parsed['user'] . (isset($parsed['pass']) ? ':' . $parsed['pass'] : '') . '@' : '';
  $uri .= isset($parsed['host']) ? $parsed['host'] : '';
  $uri .= isset($parsed['port']) ? ':' . $parsed['port'] : '';

  if (isset($parsed['path'])) {
    $uri .= (substr($parsed['path'], 0, 1) == '/') ?
      $parsed['path'] :
      ((!empty($uri) ? '/' : '' ) . $parsed['path']);
  }

  $uri .= isset($parsed['query']) ? '?' . $parsed['query'] : '';

  return $uri;
}

/**
 * Generate a request token from the request.
 */
function oauth_common_callback_request_token() {
  try {
    $req = DrupalOAuthRequest::from_request();
    $context = oauth_common_context_from_request($req);
    if (!$context) {
      throw new Exception('No OAuth context found');
    }
    $server = new DrupalOAuthServer($context);
    print $server->fetch_request_token($req);
  }
  catch (OAuthException $e) {
    drupal_add_http_header('Status', '401 Unauthorized: ' . $e->getMessage());
    drupal_add_http_header('WWW-Authenticate', sprintf('OAuth realm="%s"', url('', array('absolute' => TRUE))));
  }
}

/**
 * Get a access token for the request
 */
function oauth_common_callback_access_token() {
  try {
    $req = DrupalOAuthRequest::from_request();
    $context = oauth_common_context_from_request($req);
    if (!$context) {
      throw new Exception('No OAuth context found');
    }
    $server = new DrupalOAuthServer($context);
    $access_token = $server->fetch_access_token($req);

    // Set the expiry time based on context settings or get parameter
    $expires = !empty($context->authorization_options['access_token_lifetime']) ? REQUEST_TIME + $context->authorization_options['access_token_lifetime'] : 0;
    if ($_GET['expires'] && intval($_GET['expires'])) {
      $hint = intval($_GET['expires']);
      // Only accept more restrictive expiry times
      if ($expires == 0 || $hint < $expires) {
        $expires = $hint;
      }
    }

    // Store the expiry time if the access token should expire
    if ($expires) {
      $access_token->expires = $expires;
      $access_token->write(TRUE);
    }

    print $access_token;
  }
  catch (OAuthException $e) {
    drupal_add_http_header('Status', '401 Unauthorized: ' . $e->getMessage());
    drupal_add_http_header('WWW-Authenticate', sprintf('OAuth realm="%s"', url('', array('absolute' => TRUE))));
  }
}
