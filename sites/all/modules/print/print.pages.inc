<?php

/**
 * @file
 * Contains the functions to generate Printer-friendly pages.
 *
 * This file is included by the core PF module, and includes all the
 * functions necessary to generate a PF version of the original page
 * in HTML format.
 *
 * @ingroup print
 */

$_print_urls = PRINT_URLS_DEFAULT;

/**
 * Generate an HTML version of the printer-friendly page
 *
 * @see print_controller()
 */
function print_controller_html() {
  $args = func_get_args();
  $path = implode('/', $args);
  $cid = isset($_GET['comment']) ? (int)$_GET['comment'] : NULL;

  $print = print_controller($path, $cid, PRINT_HTML_FORMAT);
  if ($print !== FALSE) {
    $node = $print['node'];
    $html = theme('print', array('print' => $print, 'type' => PRINT_HTML_FORMAT, 'node' => $node));
    drupal_send_headers();
    print $html;

    $nodepath = (isset($node->path)) ? drupal_get_normal_path($node->path) : 'node/' . $path;
    db_merge('print_page_counter')
      ->key(array('path' => $nodepath))
      ->fields(array(
          'totalcount' => 1,
          'timestamp' => REQUEST_TIME,
      ))
      ->expression('totalcount', 'totalcount + :inc', array(':inc' => 1))
      ->execute();
  }
}

/**
 * Select the print generator function based on the page type
 *
 * Depending on the type of node, this functions chooses the appropriate
 * generator function.
 *
 * @param $path
 *   path of the original page
 * @param $cid
 *   comment ID of the individual comment to be rendered
 * @param $format
 *   format of the page being generated
 * @param $teaser
 *   if set to TRUE, outputs only the node's teaser
 * @param $message
 *   optional sender's message (used by the send e-mail module)
 * @return
 *   array with the fields to be used in the template
 * @see _print_generate_node()
 * @see _print_generate_path()
 * @see _print_generate_book()
 */
function print_controller($path, $cid = NULL, $format = PRINT_HTML_FORMAT, $teaser = FALSE, $message = NULL) {
  if (empty($path)) {
    // If no path was provided, let's try to generate a page for the referer
    global $base_url;

    $ref = $_SERVER['HTTP_REFERER'];
    $path = preg_replace("!^$base_url/!", '', $ref);
    if ($path === $ref) {
      $path = '';
    }
  }
  if ($alias = drupal_lookup_path('source', $path)) {
    // Indirect call with print/alias
    // If there is a path alias with these arguments, generate a printer-friendly version for it
    $ret = preg_match('!^node/(.*)!i', $alias, $matches);
    if ($ret == 1) {
      $path = $matches[1];
    }
  }
  $parts = explode('/', $path);
  if (ctype_digit($parts[0]) && (count($parts) == 1)) {
    $print = _print_generate_node($path, $cid, $format, $teaser, $message);
  }
  else {
    $ret = preg_match('!^book/export/html/(.*)!i', $path, $matches);
    if ($ret == 1) {
      // This is a book PF page link, handle trough the book handling functions
      $print = _print_generate_book($matches[1], $format, $teaser, $message);
    }
    else {
      // If no content node was found, handle the page printing with the 'printable' engine
      $print = _print_generate_path($path, $format, $teaser, $message);
    }
  }

  return $print;
}

/**
 * Generates a robots meta tag to tell them what they may index
 *
 * @return
 *   string with the meta robots tag
 */
function _print_robots_meta_generator() {
  $print_robots_noindex = variable_get('print_robots_noindex', PRINT_ROBOTS_NOINDEX_DEFAULT);
  $print_robots_nofollow = variable_get('print_robots_nofollow', PRINT_ROBOTS_NOFOLLOW_DEFAULT);
  $print_robots_noarchive = variable_get('print_robots_noarchive', PRINT_ROBOTS_NOARCHIVE_DEFAULT);
  $robots_meta = array();

  if (!empty($print_robots_noindex)) {
    $robots_meta[] = 'noindex';
  }
  if (!empty($print_robots_nofollow)) {
    $robots_meta[] = 'nofollow';
  }
  if (!empty($print_robots_noarchive)) {
    $robots_meta[] = 'noarchive';
  }

  if (count($robots_meta) > 0) {
    $robots_meta = implode(', ', $robots_meta);
    $robots_meta = "<meta name='robots' content='$robots_meta' />\n";
  }
  else {
    $robots_meta = '';
  }

  return $robots_meta;
}

/**
 * Post-processor that fills the array for the template with common details
 *
 * @param $node
 *   generated node with a printer-friendly node body
 * @param $message
 *   optional sender's message (used by the send e-mail module)
 * @param $cid
 *   id of current comment being generated (NULL when not generating
 *   an individual comment)
 * @return
 *   array with the fields to be used in the template
 */
function _print_var_generator($node, $message = NULL, $cid = NULL) {
  global $base_url, $language, $_print_urls;

  $path = empty($node->nid) ? $node->path : "node/$node->nid";

  // print module settings
  $print_css = variable_get('print_css', PRINT_CSS_DEFAULT);
  $print_logo_options = variable_get('print_logo_options', PRINT_LOGO_OPTIONS_DEFAULT);
  $print_logo_url = variable_get('print_logo_url', PRINT_LOGO_URL_DEFAULT);
  $print_html_new_window = variable_get('print_html_new_window', PRINT_HTML_NEW_WINDOW_DEFAULT);
  $print_html_sendtoprinter = variable_get('print_html_sendtoprinter', PRINT_HTML_SENDTOPRINTER_DEFAULT);
  $print_html_windowclose = variable_get('print_html_windowclose', PRINT_HTML_WINDOWCLOSE_DEFAULT);
  $print_sourceurl_enabled = variable_get('print_sourceurl_enabled', PRINT_SOURCEURL_ENABLED_DEFAULT);
  $print_sourceurl_forcenode = variable_get('print_sourceurl_forcenode', PRINT_SOURCEURL_FORCENODE_DEFAULT);
  $print_sourceurl_date = variable_get('print_sourceurl_date', PRINT_SOURCEURL_DATE_DEFAULT);
  $print_footer_options = variable_get('print_footer_options', PRINT_FOOTER_OPTIONS_DEFAULT);
  $print_footer_user = variable_get('print_footer_user', PRINT_FOOTER_USER_DEFAULT);

  $print['language'] = $language->language;
  $print['title'] = check_plain($node->title);
  $print['head'] = drupal_get_html_head();
  if ($print_html_sendtoprinter) {
    drupal_add_js('misc/drupal.js', array('weight' => JS_LIBRARY));
  }
  $print['scripts'] = drupal_get_js();
  $print['robots_meta'] = _print_robots_meta_generator();
  $print['url'] = url($path, array('absolute' => TRUE));
  $print['base_href'] = "<base href='" . $print['url'] . "' />\n";
  $print['favicon'] = theme_get_setting('toggle_favicon') ? "<link rel='shortcut icon' href='" . theme_get_setting('favicon') . "' type='image/x-icon' />\n" : '';

  if (!empty($print_css)) {
    drupal_add_css(strtr($print_css, array('%t' => path_to_theme())));
  }
  else {
    drupal_add_css(drupal_get_path('module', 'print') . '/css/print.css');
  }
  $drupal_css = drupal_add_css();
  foreach ($drupal_css as $key => $css_file) {
    if ($css_file['group'] == CSS_THEME) {
    // Unset the theme's CSS
      unset($drupal_css[$key]);
    }
  }

  // If we are sending a message via e-mail, the CSS must be embedded
  if (!empty($message)) {
    $style = '';
    $css_files = array_keys($drupal_css);
    foreach ($css_files as $filename) {
      $res = file_exists($filename) ? file_get_contents($filename, TRUE) : FALSE;
      if ($res != FALSE) {
        $style .= $res;
      }
    }
    $print['css'] = "<style type='text/css' media='all'>$style</style>\n";
  }
  else {
    $print['css'] = drupal_get_css($drupal_css);
  }

  $window_close = ($print_html_new_window && $print_html_windowclose) ? 'window.close();' : '';
  $print['sendtoprinter'] = $print_html_sendtoprinter ? "<script type='text/javascript'>(function ($) { Drupal.behaviors.print = { attach: function(context) {window.print();$window_close}};}(jQuery));</script>" : '';

  switch ($print_logo_options) {
    case 0: // none
      $logo_url = 0;
      break;
    case 1: // theme's
      $logo_url = theme_get_setting('logo');
      break;
    case 2: // user-specifed
      $logo_url = strip_tags($print_logo_url);
      break;
  }
  $site_name = variable_get('site_name', 'Drupal');
  $print['logo'] = $logo_url ? "<img class='print-logo' src='" . check_url($logo_url) . "' alt='$site_name' id='logo' />\n" : '';

  switch ($print_footer_options) {
    case 0: // none
      $footer = '';
      break;
    case 1: // theme's
      $footer = variable_get('site_footer', FALSE) . "\n" . drupal_render(block_get_blocks_by_region('footer'));
      break;
    case 2: // user-specifed
      $footer = $print_footer_user;
      break;
  }
  $print['footer_message'] = filter_xss_admin($footer);

  $published_site = variable_get('site_name', 0);
  if ($published_site) {
    $print_text_published = filter_xss(variable_get('print_text_published', t('Published on %site_name')));
    $published = t($print_text_published, array('%site_name' => $published_site));
    $print['site_name'] = $published . ' (' . l($base_url, $base_url) . ')';
  }
  else {
    $print['site_name'] = '';
  }

  if ($print_sourceurl_enabled == 1) {
    /* Grab and format the src URL */
    if (empty($print_sourceurl_forcenode)) {
      $url = $print['url'];
    }
    else {
      $url = $base_url . '/' . (((bool)variable_get('clean_url', '0')) ? '' : '?q=') . $path;
    }
    if (is_int($cid)) {
      $url .= '#comment-$cid';
    }
    $retrieved_date = format_date(REQUEST_TIME, 'short');
    $print_text_retrieved = filter_xss(variable_get('print_text_retrieved', t('retrieved on %date')));
    $retrieved = t($print_text_retrieved, array('%date' => $retrieved_date));
    $print['printdate'] = $print_sourceurl_date ? " ($retrieved)" : '';

    $source_url = filter_xss(variable_get('print_text_source_url', t('Source URL')));
    $print['source_url'] = '<strong>' . $source_url . $print['printdate'] . ':</strong> ' . l($url, $url);
  }
  else {
    $print['source_url'] = '';
  }

  $print['type'] = (isset($node->type)) ? $node->type : '';

  menu_set_active_item($path);
  $breadcrumb = drupal_get_breadcrumb();
  if (!empty($breadcrumb)) {
    $breadcrumb[] = menu_get_active_title();
    $print['breadcrumb'] = filter_xss(implode(' > ', $breadcrumb));
  }
  else {
    $print['breadcrumb'] = '';
  }

  // Display the collected links at the bottom of the page. Code once taken from Kjartan Mannes' project.module
  $print['pfp_links'] = '';
  if (!empty($_print_urls)) {
    $urls = _print_friendly_urls();
    $max = count($urls);
    $pfp_links = '';
    if ($max) {
      for ($i = 0; $i < $max; $i++) {
        $pfp_links .= '[' . ($i + 1) . '] ' . check_plain($urls[$i]) . "<br />\n";
      }
      $links = filter_xss(variable_get('print_text_links', t('Links')));
      $print['pfp_links'] = "<p><strong>$links:</strong><br />$pfp_links</p>";
    }
  }

  $print['node'] = $node;
  $print['message'] = $message;

  return $print;
}

/**
 * Callback function for the preg_replace_callback for URL-capable patterns
 *
 * Manipulate URLs to make them absolute in the URLs list, and to add a
 * [n] footnote marker.
 *
 * @param $matches
 *   array with the matched tag patterns, usually <a...>+text+</a>
 * @return
 *   tag with re-written URL and when appropriate the [n] index to the
 *   URL list
 */
function _print_rewrite_urls($matches) {
  global $base_url, $base_root, $_print_urls;

  $include_anchors = variable_get('print_urls_anchors', PRINT_URLS_ANCHORS_DEFAULT);

  // first, split the html into the different tag attributes
  $pattern = '!\s*(\w+\s*=\s*"(?:\\\"|[^"])*")\s*|\s*(\w+\s*=\s*\'(?:\\\\\'|[^\'])*\')\s*|\s*(\w+\s*=\s*\w+)\s*|\s+!';
  $attribs = preg_split($pattern, $matches[1], -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
  foreach ($attribs as $key => $value) {
    $attribs[$key] = preg_replace('!(\w)\s*=\s*(.*)!', '$1=$2', $value);
  }

  $size = count($attribs);
  for ($i=1; $i < $size; $i++) {
    // If the attribute is href or src, we may need to rewrite the URL in the value
    if (preg_match('!^(?:href|src)\s*?=(.*)!i', $attribs[$i], $urls) > 0) {
      $url = trim($urls[1], " \t\n\r\0\x0B\"'");

      if (strpos(html_entity_decode($url), '://') || preg_match('!^mailto:.*?@.*?\..*?$!iu', html_entity_decode($url))) {
        // URL is absolute, do nothing
        $newurl = $url;
      }
      else {
        if ($url[0] == '#') {
          // URL is an anchor tag
          if ($include_anchors && (!empty($_print_urls))) {
            $path = explode('/', $_GET['q']);
            unset($path[0]);
            $path = implode('/', $path);
            if (ctype_digit($path)) {
              $path = "node/$path";
            }
            // Printer-friendly URLs is on, so we need to make it absolute
            $newurl = url($path, array('fragment' => substr($url, 1), 'absolute' => TRUE));
          }
          // Because base href is the original page, change the link to
          // still be usable inside the print page
          $matches[1] = str_replace($url, base_path() . $_GET['q'] . $url, $matches[1]);
        }
        else {
          // URL is relative, convert it into absolute URL
          if ($url[0] == '/') {
            // If it starts with '/' just append it to the server name
            $newurl = $base_root . '/' . trim($url, '/');
          }
          elseif (preg_match('!^(?:index.php)?\?q=!i', $url)) {
            // If it starts with ?q=, just prepend with the base URL
            $newurl = $base_url . '/' . trim($url, '/');
          }
          else {
            $newurl = url(trim($url, '/'), array('absolute' => TRUE));
          }
          $matches[1] = str_replace($url, $newurl, $matches[1]);
        }
      }
    }
  }

  $ret = '<' . $matches[1] . '>';
  if (count($matches) == 4) {
    $ret .= $matches[2] . $matches[3];
    if ((!empty($_print_urls)) && (isset($newurl))) {
      $ret .= ' <span class="print-footnote">[' . _print_friendly_urls(trim($newurl)) . ']</span>';
    }
  }

  return $ret;
}

/**
 * Auxiliary function to store the Printer-friendly URLs list as static.
 *
 * @param $url
 *   absolute URL to be inserted in the list
 * @return
 *   list of URLs previously stored if $url is 0, or the current count
 *   otherwise.
 */
function _print_friendly_urls($url = 0) {
  static $urls = array();
  if ($url) {
    $url_idx = array_search($url, $urls);
    if ($url_idx !== FALSE) {
      return ($url_idx + 1);
    }
    else {
      $urls[] = $url;
      return count($urls);
    }
  }
  $ret = $urls;
  $urls = array();
  return $ret;
}

/**
 * Check URL list settings for this node
 *
 * @param node
 *   node object
 * @param $format
 *   format of the page being generated
 * @return
 *   TRUE if URL list should be displayed, FALSE otherwise
 */
function _print_url_list_enabled($node, $format = PRINT_HTML_FORMAT) {
  if (!isset($node->type)) {
    switch ($format) {
      case PRINT_HTML_FORMAT:
        $node_urllist = variable_get('print_display_sys_urllist', PRINT_TYPE_SYS_URLLIST_DEFAULT);
        break;
      case PRINT_MAIL_FORMAT:
        $node_urllist = variable_get('print_mail_display_sys_urllist', PRINT_TYPE_SYS_URLLIST_DEFAULT);
        break;
      case PRINT_PDF_FORMAT:
        $node_urllist = variable_get('print_pdf_display_sys_urllist', PRINT_TYPE_SYS_URLLIST_DEFAULT);
        break;
      default:
        $node_urllist = PRINT_TYPE_SYS_URLLIST_DEFAULT;
    }
  }
  else {
    switch ($format) {
      case PRINT_HTML_FORMAT:
        $node_urllist = isset($node->print_display_urllist) ? $node->print_display_urllist : variable_get('print_display_urllist_' . $node->type, PRINT_TYPE_URLLIST_DEFAULT);
        break;
      case PRINT_MAIL_FORMAT:
        $node_urllist = isset($node->print_mail_display_urllist) ? $node->print_mail_display_urllist : variable_get('print_mail_display_urllist_' . $node->type, PRINT_TYPE_URLLIST_DEFAULT);
        break;
      case PRINT_PDF_FORMAT:
        $node_urllist = isset($node->print_pdf_display_urllist) ? $node->print_pdf_display_urllist : variable_get('print_pdf_display_urllist_' . $node->type, PRINT_TYPE_URLLIST_DEFAULT);
        break;
      default:
        $node_urllist = PRINT_TYPE_URLLIST_DEFAULT;
    }
  }

  // Get value of Printer-friendly URLs setting
  return (variable_get('print_urls', PRINT_URLS_DEFAULT) && ($node_urllist));
}

/**
 * Prepare a Printer-friendly-ready node body for content nodes
 *
 * @param $nid
 *   node ID of the node to be rendered into a printer-friendly page
 * @param $cid
 *   comment ID of the individual comment to be rendered
 * @param $format
 *   format of the page being generated
 * @param $teaser
 *   if set to TRUE, outputs only the node's teaser
 * @param $message
 *   optional sender's message (used by the send e-mail module)
 * @return
 *   filled array ready to be used in the template
 */
function _print_generate_node($nid, $cid = NULL, $format = PRINT_HTML_FORMAT, $teaser = FALSE, $message = NULL) {
  global $_print_urls;

  if (!isset($langcode)) {
    $langcode = $GLOBALS['language_content']->language;
  }

  // We can take a node id
  $node = node_load($nid);
  if (!$node) {
    // Node not found
    drupal_not_found();
    return FALSE;
  }
  elseif (!node_access('view', $node)) {
    // Access is denied
    drupal_access_denied();
    return FALSE;
  }
  drupal_set_title($node->title);

  $view_mode = $teaser ? 'teaser' : 'print';

  // Turn off Pagination by the Paging module
  unset($node->pages);
  unset($node->page_count);

  // Make this page a member of the original page's organic group
  if (function_exists('og_set_group_context') && isset($node->og_groups)) {
    og_set_group_context($node->og_groups);
  }

  if ($cid === NULL) {
    // Adapted (simplified) version of node_view
    //Render the node content
    node_build_content($node, $view_mode);

    // Disable the links area
    unset($node->content['links']);
    // Disable fivestar widget output
    unset($node->content['fivestar_widget']);
    // Disable service links module output
    unset($node->content['service_links']);

    $build = $node->content;
    unset($node->content);

    // Disable the AdSense module ads
//    $content = preg_replace('!<div class=[\'"]adsense[\'"].*?</div>!sim', '', $content);
  }

  $print_comments = variable_get('print_comments', PRINT_COMMENTS_DEFAULT);

  if (function_exists('comment_render') && (($cid != NULL) || ($print_comments))) {
    //Print only the requested comment (or if $cid is NULL, all of them)
    $comments = comment_render($node, $cid);

    //Remove the comment forms
    $comments = preg_replace('!<form.*?id="comment-.*?">.*?</form>!sim', '', $comments);
    //Remove the 'Post new comment' title
    $comments = preg_replace('!<h2.*?>' . t('Post new comment') . '</h2>!', '', $comments);
    //Remove the comment title hyperlink
    $comments = preg_replace('!(<h3.*?>.*?)<a.*?>(.*?)</a>(.*?</h3>)!i', '$1$2$3', $comments);
    //Remove the comment author link
    $pattern = '!(<(?:span|div) class="submitted">.*?)<a.*?>(.*?)</a>(.*?</(?:span|div)>)!sim';
    if (preg_match($pattern, $comments)) {
      $comments = preg_replace($pattern , '$1$2$3', $comments);
    }
    //Remove the comment links
    $comments = preg_replace('!\s*<ul class="links">.*?</ul>!sim', '', $comments);
    if ($cid != NULL) {
      // Single comment requested, output only the comment
      unset($node->body);
    }
    $node->body .= $comments;
  }

  $build += array(
    '#theme' => 'node',
    '#node' => $node,
    '#view_mode' => $view_mode,
    '#language' => $langcode,
  );

  drupal_alter('node_view', $build);

  $content = render($build);

  //Get rid of the annoying D7 title link
  $pattern = '!(<h2 property="dc:title" datatype="">)\s*<a [^>]*?>(.*?)</a>\s*(</h2>)!mis';
  $content = preg_replace($pattern, '$1$2$3', $content);

  //Check URL list settings
  $_print_urls = _print_url_list_enabled($node, $format);

  // Convert the a href elements
  $pattern = '!<(a\s[^>]*?)>(.*?)(</a>)!is';
  $content = preg_replace_callback($pattern, '_print_rewrite_urls', $content);

  $print = _print_var_generator($node, $message, $cid);
  $print['content'] = $content;

  return $print;
}

/**
 * Prepare a Printer-friendly-ready node body for non-content pages
 *
 * @param $path
 *   path of the node to be rendered into a printer-friendly page
 * @param $format
 *   format of the page being generated
 * @param $teaser
 *   if set to TRUE, outputs only the node's teaser
 * @param $message
 *   optional sender's message (used by the send e-mail module)
 * @return
 *   filled array ready to be used in the template
 */
function _print_generate_path($path, $format = PRINT_HTML_FORMAT, $teaser = FALSE, $message = NULL) {
  global $_print_urls;

  // Handle node tabs
  $parts = explode('/', $path);
  if (ctype_digit($parts[0]) && (count($parts) > 1)) {
    $path = 'node/' . $path;
  }

  $path = drupal_get_normal_path($path);

  menu_set_active_item($path);
  // Adapted from index.php.
  $node = new stdClass();
  $node->body = menu_execute_active_handler($path, FALSE);
  if (is_array($node->body)) {
    $node->body = drupal_render($node->body);
  }

  if (is_int($node->body)) {
    switch ($node->body) {
      case MENU_NOT_FOUND:
        drupal_not_found();
        return FALSE;
        break;
      case MENU_ACCESS_DENIED:
        drupal_access_denied();
        return FALSE;
        break;
    }
  }

  $node->title = drupal_get_title();
  $node->path = $path;
  $node->changed = 0;

  // Delete any links area
  $node->body = preg_replace('!\s*<div class="links">.*?</div>!sim', '', $node->body);

  //Check URL list settings
  $_print_urls = _print_url_list_enabled($node, $format);

  // Convert the a href elements
  $pattern = '!<(a\s[^>]*?)>(.*?)(</a>)!is';
  $node->body = preg_replace_callback($pattern, '_print_rewrite_urls', $node->body);

  $print = _print_var_generator($node, $message);
  $print['content'] = $node->body;

  return $print;
}


/**
 * Prepare a Printer-friendly-ready node body for book pages
 *
 * @param $nid
 *   node ID of the node to be rendered into a printer-friendly page
 * @param $format
 *   format of the page being generated
 * @param $teaser
 *   if set to TRUE, outputs only the node's teaser
 * @param $message
 *   optional sender's message (used by the send e-mail module)
 * @return
 *   filled array ready to be used in the template
 */
function _print_generate_book($nid, $format = PRINT_HTML_FORMAT, $teaser = FALSE, $message = NULL) {
  global $_print_urls;

  $node = node_load($nid);
  if (!$node) {
    // Node not found
    drupal_not_found();
    return FALSE;
  }
  elseif (!node_access('view', $node) || (!user_access('access printer-friendly version'))) {
    // Access is denied
    drupal_access_denied();
    return FALSE;
  }

  $tree = book_menu_subtree_data($node->book);
  $node->body = book_export_traverse($tree, 'book_node_export');

  //Check URL list settings
  $_print_urls = _print_url_list_enabled($node, $format);

  // Convert the a href elements
  $pattern = '!<(a\s[^>]*?)>(.*?)(</a>)!is';
  $node->body = preg_replace_callback($pattern, '_print_rewrite_urls', $node->body);

  $print = _print_var_generator($node, $message);
  $print['content'] = $node->body;

  // The title is already displayed by the book_recurse, so avoid duplication
  $print['title'] = '';

  return $print;
}
