/*
 *  I'd like to start by giving credit where its due.
 *  Most of this documentation was designed based on:
 *
 *  http://mydrupalblog.lhmdesign.com/drupal-theming-jquery-basics-inc-drupal-behaviors
 * 
 *
 *   Please keep in mind.  This code will NOT work for YOU.
 *
 *  Why not?
 *
 *    Simple.  This is configured for the site it is ON.
 *
 *  "Can I make it work for my site?"
 *
 *    Why yes, yes you can.  Just add the CSS ID 'toggleoptions'
 *    to your .css and create this file.  Viola!
 *
 *  "But where do I put the javascript?"
 *
 *    Mine are in:   /sites/all/themes/charlessubtheme/js/
 *
 *  Javascript?  What javascript.
 *  
 *    This is a javascript.  I'd suggest just naming it script.js
 *     Drupal will automatically pickup the file.
 *
 *  Really?
 *
 *    Well no, not really.  There are three options for inserting your .js
 *    file into a Drupal build.
 *
 *    1. Open your my_theme.info file and add the line:
 *         scripts[] = my_scripts.js
 *    2. Add the following lines to your template.php file
 *    (See alpha theme's template for an example)
 *    drupal_add_js(drupal_get_path('theme', 'my_theme_name') . '/js/my_scripts.js', 'theme');
 */  


/*  toggle Options
 *  The toggle function will be used throughout the site
 *  However, this particular instance is in the version 1 tutorial.
 *
 *
 */ 
 
 
Drupal.behaviors.toggleoptions = function (context) {
    $('a#toggleoptions-link:not(.toggleoptions-processed)', context).addClass('toggleoptions-processed').each(function () {
      $(this).click(function() {
        $("div#toggleoptions").toggle(400);
        return false;
      });
    });
  }(jQuery);
};

/*
 *  The Breakdown:
 *
 *   If you are using a text-editor such as Notepad++ or Xcode,
 *   it might be easiest to just uncomment this section to see 
 *   the step by step breaddown of the code.
 *
 *   Step 1:  Drupal.behaviors.toggle = function (context) {
 *
 *   Here we are creating a Drupal Behavior and dropping it into the 'context' object.
 *   Well get into both of those later in more depth.  This is a "how to" tutorial
 *   much more than a "what is" tutorial.
 *
 *   Step 2: $('a#toggleoptions-link:not(.toggleoptions-processed)', context).addClass('toggleoptions-processed').each(function () {   }); 
 *  
 *   Basically, we're diving through the CSS to find anything that has
 *   a CSS id named " toggleoptions-processed " somewhere within your associated
 *   content (after adding the id to 'play' within the function.
 *
 *   aka:   id = "toggleoptions-link"  
 *
 *
 *   The computer actual reads it in the following (VERY) loose translation:
 *   
 *    1. a#toggleoptions-link <--- Search for toggleoptions-link
 *      1a. :not()   <---- Check to see if #1 is NOT processed.
 *      1b. If it is NOT processed, at -processed to it, and process the function.
 *          ( Note - Process the function = 'do the action for this function' )  
 *
 *   
 *
 *  Step 3:  $(this).click(function() {    });
 *    
 *  This one is REALLY easy to understand
 *
 *    1. Add a .click function.  aka:  Click for an action to occur.
 *
 *    That action fires when......
 *
 *  Step 4:  $("div #toggleoptions").toggle(400);
 *
 *    aka:  When somebody clicks (step 3), it toggles the div off at 400 milliseconds.
 *
 *  Step 5:  return false;
 *  
 *  This one is pretty much base generic-code. 
 *
 *  Do not let the clicked link 'fire'.  Thus, the link doesn't actually work.
 *  Instead, we have overriden the HTML code for 'jump to this link' to be
 *  'toggle and show the stuff'.
 *
 *   Congratulations!  You've done your first jQuery action.
 *
 *
 *   Later we will get into (if you have trouble figuring it out) how to
 *    actually incorporate jQuery directly into your themes.
 *
 *   Step 6:  Wait, what?  There's no more code!
 *
 *
 *     Yes, I know there isn't.  Step six is to clear your cache.
 *     Biggest newbie mistake, hands down.
 */
 
 /*
  *  Easy little extra to get you started.
  *
  *  Fade in all of the links on the site slowly.
  *
  *  This should help out if you need it.
  */ 

 
 
 // Mkae this shit functional
 
 $(document).ready(function(){
   alert("Hellow World");
 
 });
   
   
   
   
   
   
   
   
   
   
   
   
 
 