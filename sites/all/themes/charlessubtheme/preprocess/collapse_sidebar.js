/** 
 *  Hides Sidebars
 *
 */



(function($) {
$('div.block h2').click(function() {
  $(this).parent().find('div.content').slideToggle();
});
})(jQuery);

