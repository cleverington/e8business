<?php
/**
 * Field handler to provide acess control for the email field
 */
class views_handler_field_user_mail extends views_handler_field_user {
  function option_definition() {
    $options = parent::option_definition();
    $options['link_to_user'] = array('default' => 'mailto');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['link_to_user'] = array(
      '#title' => t('Link this field'),
      '#type' => 'radios',
      '#options' => array(
        0 => t('No link'),
        'user' => t('To the user'),
        'mailto' => t("With a mailto:"),
      ),
      '#default_value' => $this->options['link_to_user'],
    );
    parent::options_form($form, $form_state);
  }

  function render_link($data, $values) {
    parent::render_link($data, $values);

    if ($this->options['link_to_user'] == 'mailto') {
      $this->options['alter']['make_link'] = TRUE;
      $this->options['alter']['path'] = "mailto:" . $data;
    }

    return $data;
  }
}
