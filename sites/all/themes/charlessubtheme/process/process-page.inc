<?php

  /* This tells the entire Drupal site that we are using the jQuery UI for
  *    whatever 'action' is about to fire.
  *
  */
  drupal_add_library('system', 'ui.dialog');

  
  /*
  *  If browser sees #dialog somewhere in a div, box it, toss it, and throw it out
  *    on a boxed overlay.  This 'action' should even fire on hidden elements.
  *    Will test later.  Think survey question pop-ups.
  */
  drupal_add_js('jQuery(document).ready(function(){
    jQuery("#dialog").dialog({ modal: true }); });', 'inline');
   
  
  drupal_add_js('jQuery(document).ready(function(){
    jQuery("a").fadeIn(400); });', 'inline');

?>