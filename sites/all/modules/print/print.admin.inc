<?php

/**
 * @file
 * Contains the administrative functions of the PF module.
 *
 * This file is included by the core PF module, and includes the
 * settings form.
 *
 * @ingroup print
 */

/**
 * Menu callback for the Printer-friendly pages module settings form.
 *
 * @ingroup forms
 */
function print_main_settings() {
  $print_css = variable_get('print_css', PRINT_CSS_DEFAULT);
  // Check that the stylesheet path is indeed valid and relative to base_path()
  if (!empty($print_css)) {
    $css_path = dirname($_SERVER['SCRIPT_FILENAME']) . '/' . strtr($print_css, array('%t' => drupal_get_path('theme', variable_get('theme_default', 'garland'))));
    if (!is_file($css_path) || !is_readable($css_path)) {
      drupal_set_message(t("File %file is not readable by the web server.", array('%file' => $css_path)), 'warning', FALSE);
    }
  }

  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Common Settings'),
  );

  $form['settings']['print_css'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom Stylesheet'),
    '#default_value' => variable_get('print_css', PRINT_CSS_DEFAULT),
    '#size' => 60,
    '#maxlength' => 250,
    '#description' => t('The path to your custom print cascading stylesheet, if any. The path provided must be relative to the base path. When none is specified, the default module CSS file is used.') . '<br />' .
                      t('Macros: %t (path to theme: "%theme")', array('%theme' => drupal_get_path('theme', variable_get('theme_default', 'bartik')))) . '<br />' .
                      t('Requires the <em>administer site configuration</em> permission.'),
    '#disabled' => !user_access('administer site configuration'),
  );

  $form['settings']['print_urls'] = array(
    '#type' => 'checkbox',
    '#title' => t('Printer-friendly URLs list'),
    '#default_value' => variable_get('print_urls', PRINT_URLS_DEFAULT),
    '#description' => t('If set, a list of the destination URLs for the page links will be displayed at the bottom of the page.') . '<br />' .
                      t('Note that you can enable/disable the URL list for each content type individually from the !url.', array('!url' => l(t('Content Types page'), 'admin/structure/types'))),
  );

  $form['settings']['print_urls_anchors'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include named anchors in printer-friendly URLs list'),
    '#default_value' => variable_get('print_urls_anchors', PRINT_URLS_ANCHORS_DEFAULT),
    '#description' => t('If set, the list of the URLs at the bottom of the page will include anchors links on the same page.'),
  );

  $form['settings']['print_comments'] = array(
    '#type' => 'checkbox',
    '#title' => t('Include comments in printer-friendly version'),
    '#default_value' => variable_get('print_comments', PRINT_COMMENTS_DEFAULT),
    '#description' => t('When this option is active, user comments are also included in the printer-friendly version. Requires the comment module.'),
  );

  $form['settings']['print_newwindow'] = array(
    '#type' => 'radios',
    '#title' => t('New window method'),
    '#options' => array(t('Use HTML target (does not validate as XHTML Strict)'), t('Use Javascript (requires browser support)')),
    '#default_value' => variable_get('print_newwindow', PRINT_NEWWINDOW_DEFAULT),
    '#description' => t('Choose the method used to open pages in a new window/tab.'),
  );

  $form['settings']['logo'] = array(
    '#type' => 'fieldset',
    '#title' => t('Logo options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['settings']['logo']['print_logo_options'] = array(
    '#type' => 'radios',
    '#title' => t('Logo type'),
    '#options' => array(t('None (Disabled)'), t("Current theme's logo"), t('User-specified')),
    '#default_value' => variable_get('print_logo_options', PRINT_LOGO_OPTIONS_DEFAULT),
    '#description' => t("Select the type of logo to display on the printer-friendly version. In case of a user-specified location, insert the path or URL below."),
  );

  $form['settings']['logo']['print_logo_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Logo URL'),
    '#default_value' => variable_get('print_logo_url', PRINT_LOGO_URL_DEFAULT),
    '#size' => 60,
    '#maxlength' => 250,
  );

  $form['settings']['logo']['print_logo_upload'] = array(
    '#type' => 'file',
    '#title' => t('Upload logo'),
    '#size' => 60,
    '#maxlength' => 250,
    '#description' => t("If you don't have direct file access to the server, use this field to upload your logo."),
  );

  $form['settings']['footer'] = array(
    '#type' => 'fieldset',
    '#title' => t('Footer options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['settings']['footer']['print_footer_options'] = array(
    '#type' => 'radios',
    '#title' => t('Footer type'),
    '#options' => array(t('None (Disabled)'), t("Site's footer"), t('User-specified')),
    '#default_value' => variable_get('print_footer_options', PRINT_FOOTER_OPTIONS_DEFAULT),
    '#description' => t("Select the type of footer to display on the printer-friendly version. In case of a user-specified footer, insert it below."),
  );

  $form['settings']['footer']['print_footer_user'] = array(
    '#type' => 'textfield',
    '#title' => t('User-specified'),
    '#default_value' => variable_get('print_footer_user', PRINT_FOOTER_USER_DEFAULT),
    '#size' => 60,
    '#maxlength' => 250,
  );

  $form['settings']['print_sourceurl_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Source URL'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['settings']['print_sourceurl_settings']['print_sourceurl_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display source URL'),
    '#default_value' => variable_get('print_sourceurl_enabled', PRINT_SOURCEURL_ENABLED_DEFAULT),
    '#description' => t('When this option is selected, the URL for the original page will be displayed at the bottom of the printer-friendly version.'),
  );

  $form['settings']['print_sourceurl_settings']['print_sourceurl_date'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add current time/date to the source URL'),
    '#default_value' => variable_get('print_sourceurl_date', PRINT_SOURCEURL_DATE_DEFAULT),
    '#description' => t('Display the current date and time in the Source URL line.'),
  );

  $form['settings']['print_sourceurl_settings']['print_sourceurl_forcenode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Force use of node ID in source URL'),
    '#default_value' => variable_get('print_sourceurl_forcenode', PRINT_SOURCEURL_FORCENODE_DEFAULT),
    '#description' => t("Drupal will attempt to use the page's defined alias in case there is one. To force the use of the fixed URL, activate this option."),
  );

  $form['#attributes'] = array('enctype' => 'multipart/form-data');
  $form['#validate'][] = '_print_main_settings_validate';

  return system_settings_form($form);
}

/**
 * Validate print_main_settings form.
 */
function _print_main_settings_validate($form, &$form_state) {
  global $base_url;

  // Check for a new uploaded logo, and use that instead.
  if ($file = file_save_upload('print_logo_upload', array('file_validate_is_image' => array()))) {
    $parts = pathinfo($file->filename);
    $filename = 'print_logo.' . drupal_strtolower($parts['extension']);

    // The image was saved using file_save_upload() and was added to the
    // files table as a temporary file. We'll make a copy and let the garbage
    // collector delete the original upload.
    if (file_unmanaged_copy($file->filename, $filename, FILE_EXISTS_REPLACE)) {
      $form_state['values']['print_logo_options'] = 2;
      $form_state['values']['print_logo_url'] = base_path() . $file->filepath;
    }
  }
}

/**
 * Menu callback for the Printer-friendly pages HTML settings form.
 *
 * @ingroup forms
 */
function print_html_settings() {
  $form['settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Web page options'),
  );

  $form['settings']['print_html_link_pos'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Printer-friendly page link'),
    '#default_value' => variable_get('print_html_link_pos', array(PRINT_HTML_LINK_POS_DEFAULT => PRINT_HTML_LINK_POS_DEFAULT)),
    '#options' => array('link' => t('Links area'), 'corner' => t('Content corner')),
    '#description' => t('Choose the location of the link(s) to the printer-friendly page. The Links area is usually below the node content, whereas the Content corner is placed in the upper-right corner of the node content. Unselect all options to disable the link. Even if the link is disabled, you can still view the print version of a node by going to !path/nid where nid is the numeric id of the node.', array('!path' => PRINT_PATH)),
  );

  $form['settings']['print_html_link_teaser'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display printer-friendly link in teaser'),
    '#default_value' => variable_get('print_html_link_teaser', PRINT_HTML_LINK_TEASER_DEFAULT),
    '#description' => t('Enabling this will display the link in teaser mode.'),
  );

  $form['settings']['adv_link'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced link options'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['settings']['adv_link']['print_html_show_link'] = array(
    '#type' => 'radios',
    '#title' => t('Link style'),
    '#default_value' => variable_get('print_html_show_link', PRINT_HTML_SHOW_LINK_DEFAULT),
    '#options' => array(1 => t('Text only'), 2 => t('Icon only'), 3 => t('Icon and Text')),
    '#description' => t('Select the visual style of the link.'),
  );

  $form['settings']['adv_link']['print_html_link_use_alias'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use URL alias instead of node ID'),
    '#default_value' => variable_get('print_html_link_use_alias', PRINT_HTML_LINK_USE_ALIAS_DEFAULT),
    '#description' => t('Enabling this will create the link using the URL alias instead of the node ID.'),
  );

  $form['settings']['adv_link']['print_html_link_class'] = array(
    '#type' => 'textfield',
    '#title' => t('Link class'),
    '#default_value' => variable_get('print_html_link_class', PRINT_HTML_LINK_CLASS_DEFAULT),
    '#size' => 60,
    '#maxlength' => 250,
    '#description' => t('This can be used by themers to change the link style or by jQuery modules to open in a new window (e.g. greybox or thickbox). Multiple classes can be specified, separated by spaces.'),
  );

  $form['settings']['adv_link']['print_html_node_link_visibility'] = array(
    '#type' => 'radios',
    '#title' => t('Link visibility'),
    '#default_value' => variable_get('print_html_node_link_visibility', PRINT_HTML_NODE_LINK_VISIBILITY_DEFAULT),
    '#options' => array(t('Show on every page except the listed pages.'), t('Show on only the listed pages.')),
  );

  $form['settings']['adv_link']['print_html_node_link_pages'] = array(
    '#type' => 'textarea',
    '#default_value' => variable_get('print_html_node_link_pages', PRINT_HTML_NODE_LINK_PAGES_DEFAULT),
    '#rows' => 3,
    '#description' => t("Enter one page per line as Drupal paths. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>')),
  );

  if (module_exists('php')) {
    $access = user_access('use PHP for settings');

    if ($form['settings']['adv_link']['print_html_node_link_visibility']['#default_value'] == 2 && !$access) {
      $form['settings']['adv_link']['print_html_node_link_visibility'] = array('#type' => 'value', '#value' => 2);
      $form['settings']['adv_link']['print_html_node_link_pages'] = array('#type' => 'value', '#value' => $form['settings']['adv_link']['print_html_node_link_pages']['#default_value']);
    }
    elseif ($access) {
      $form['settings']['adv_link']['print_html_node_link_visibility']['#options'][] = t('Show if the following PHP code returns <code>TRUE</code> (PHP-mode, experts only).');
      $form['settings']['adv_link']['print_html_node_link_pages']['#description'] .= ' ' . t('If the PHP-mode is chosen, enter PHP code between %php. Note that executing incorrect PHP-code can break your Drupal site.', array('%php' => '<?php ?>'));
    }
  }

  $form['settings']['adv_link']['print_html_sys_link_visibility'] = array(
    '#type' => 'radios',
    '#title' => t('Show link in system (non-content) pages'),
    '#default_value' => variable_get('print_html_sys_link_visibility', PRINT_HTML_SYS_LINK_VISIBILITY_DEFAULT),
    '#options' => array(t('Show on every page except the listed pages.'), t('Show on only the listed pages.')),
  );

  $form['settings']['adv_link']['print_html_sys_link_pages'] = array(
    '#type' => 'textarea',
    '#default_value' => variable_get('print_html_sys_link_pages', PRINT_HTML_SYS_LINK_PAGES_DEFAULT),
    '#rows' => 3,
    '#description' => t('Setting this option will add a printer-friendly version page link on pages created by Drupal or the enabled modules.') . '<br />' .
                      t("Enter one page per line as Drupal paths. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>')),
  );

  if (module_exists('php')) {
    if ($form['settings']['adv_link']['print_html_sys_link_visibility']['#default_value'] == 2 && !$access) {
      $form['settings']['adv_link']['print_html_sys_link_visibility'] = array('#type' => 'value', '#value' => 2);
      $form['settings']['adv_link']['print_html_sys_link_pages'] = array('#type' => 'value', '#value' => $form['settings']['adv_link']['print_html_sys_link_pages']['#default_value']);
    }
    elseif ($access) {
      $form['settings']['adv_link']['print_html_sys_link_visibility']['#options'][] = t('Show if the following PHP code returns <code>TRUE</code> (PHP-mode, experts only).');
      $form['settings']['adv_link']['print_html_sys_link_pages']['#description'] .= ' ' . t('If the PHP-mode is chosen, enter PHP code between %php. Note that executing incorrect PHP-code can break your Drupal site.', array('%php' => '<?php ?>'));
    }
  }

  $form['settings']['adv_link']['print_html_book_link'] = array(
    '#type' => 'radios',
    '#title' => t('Link in book hierarchy nodes'),
    '#default_value' => variable_get('print_html_book_link', PRINT_HTML_BOOK_LINK_DEFAULT),
    '#options' => array(t('Book module link not modified'), t('Current page and sub-pages'), t('Current page only')),
  );

  $form['settings']['print_html_new_window'] = array(
    '#type' => 'checkbox',
    '#title' => t('Open the printer-friendly version in a new window'),
    '#default_value' => variable_get('print_html_new_window', PRINT_HTML_NEW_WINDOW_DEFAULT),
    '#description' => t('Setting this option will make the printer-friendly version open in a new window/tab.'),
  );

  $form['settings']['print_html_sendtoprinter'] = array(
    '#type' => 'checkbox',
    '#title' => t('Send to printer'),
    '#default_value' => variable_get('print_html_sendtoprinter', PRINT_HTML_SENDTOPRINTER_DEFAULT),
    '#description' => t("Automatically calls the browser's print function when the printer-friendly version is displayed."),
  );

  $form['settings']['print_html_windowclose'] = array(
    '#type' => 'checkbox',
    '#title' => t('Close window after sending to printer'),
    '#default_value' => variable_get('print_html_windowclose', PRINT_HTML_WINDOWCLOSE_DEFAULT),
    '#description' => t("When the above options are enabled, this option will close the window after its contents are printed."),
  );

  $form['settings']['print_display_sys_urllist'] = array(
    '#type' => 'checkbox',
    '#title' => t('Printer-friendly URLs list in system pages'),
    '#default_value' => variable_get('print_display_sys_urllist', PRINT_TYPE_SYS_URLLIST_DEFAULT),
    '#description' => t('Enabling this option will display a list of printer-friendly destination URLs at the bottom of the page.'),
  );

  $form['settings']['print_robots_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Robots META tags'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
  );

  $form['settings']['print_robots_settings']['print_robots_noindex'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add noindex'),
    '#default_value' => variable_get('print_robots_noindex', PRINT_ROBOTS_NOINDEX_DEFAULT),
    '#description' => t('Instruct robots to not index printer-friendly pages. Recommended for good search engine karma.'),
  );

  $form['settings']['print_robots_settings']['print_robots_nofollow'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add nofollow'),
    '#default_value' => variable_get('print_robots_nofollow', PRINT_ROBOTS_NOFOLLOW_DEFAULT),
    '#description' => t('Instruct robots to not follow outgoing links on printer-friendly pages.'),
  );

  $form['settings']['print_robots_settings']['print_robots_noarchive'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add noarchive'),
    '#default_value' => variable_get('print_robots_noarchive', PRINT_ROBOTS_NOARCHIVE_DEFAULT),
    '#description' => t('Non-standard tag to instruct search engines to not show a "Cached" link for your printer-friendly pages. Recognized by Googlebot.'),
  );

  return system_settings_form($form);
}

/**
 * Menu callback for the Printer-friendly pages module text strings settings form.
 *
 * @ingroup forms
 */
function print_main_strings_settings() {
  drupal_set_message(t("Saving these strings will disable their translation via Drupal's language system. Use the reset button to return them to the original state."), 'warning');

  $form['print_main_text'] = array(
    '#type' => 'fieldset',
    '#title' => t('Text strings'),
  );

  $form['print_main_text']['print_text_published'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('print_text_published', t('Published on %site_name')),
  );
  $form['print_main_text']['print_text_source_url'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('print_text_source_url', t('Source URL')),
  );
  $form['print_main_text']['print_text_retrieved'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('print_text_retrieved', t('retrieved on %date')),
  );
  $form['print_main_text']['print_text_links'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('print_text_links', t('Links')),
  );
  $form['print_main_text']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to defaults'),
    '#submit' => array('print_main_strings_settings_delete'),
  );

  return system_settings_form($form);
}

/**
 * Reset button callback for text strings settings form
 *
 * @ingroup forms
 */
function print_main_strings_settings_delete() {
  variable_del('print_text_published');
  variable_del('print_text_source_url');
  variable_del('print_text_retrieved');
  variable_del('print_text_links');
}

/**
 * Menu callback for the Printer-friendly pages module text strings settings form.
 *
 * @ingroup forms
 */
function print_html_strings_settings() {
  drupal_set_message(t("Saving these strings will disable their translation via Drupal's language system. Use the reset button to return them to the original state."), 'warning');

  $form['print_html_text'] = array(
    '#type' => 'fieldset',
    '#title' => t('Text strings'),
  );

  $form['print_html_text']['print_html_link_text'] = array(
    '#type' => 'textfield',
    '#title' => t('Link text'),
    '#default_value' => variable_get('print_html_link_text', t('Printer-friendly version')),
    '#description' => t('Text used in the link to the printer-friendly version.'),
  );
  $form['print_html_text']['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset to defaults'),
    '#submit' => array('print_html_strings_settings_delete'),
  );

  return system_settings_form($form);
}

/**
 * Reset button callback for text strings settings form
 *
 * @ingroup forms
 */
function print_html_strings_settings_delete() {
  variable_del('print_html_link_text');
}
