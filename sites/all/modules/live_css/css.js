(function($){

//main css styler management object
window.styler = {
    html: "<div class='controls'><select class='stylelist' /><div class='save'>Save</div><div class='close'>Close</div><div class='clear:both'></div></div>",
    styleObjs: {},
    sList: [],
    init: function(){
        
        //create ui
        $('#csidebar').append(this.html);
        
        var self = this;
        $('#csidebar .stylelist').change(function(){
            self.selectStyle($(this).val(), function(css){
                editor.setText(css);
            });
        });
        $('#csidebar .save').click(function(){
            $(this).html('...');
            self.saveCSS(); 
        });
        $('#csidebar .close').click(function(){
            sidebar.remove();
        });
        
        
        this.loadStyleSheets();
        
        self.selectStyle($('#csidebar .stylelist').val(), function(css){
            //create the editor
            editor.init({
                css: css,
                change: function(){
                  if(self.styleObjs[self.curStyle] != null)
                    self.styleObjs[self.curStyle].setStyle(editor.getText());
                }
            });
        });

    },
    loadStyleSheets: function(){
        var addItem = function(item){
            //remove query string
            var title = item.split('?')[0];    
            //remove http
            title = title.replace(/http:\/\//, '');
        
            //normalise path to base url
            var parts = title.split('/');
            title = '';
            var rootIndex = Drupal.settings.basePath.split('/').length - 1;
            for(var i = rootIndex; i < parts.length; i++){
                title = title + parts[i] + '/';
            }
            title = title.substr(0, title.length - 1);
            
            //limit to just theme css if necessary
            if(Drupal.settings.live_css.hidemodules){
                if(!title.match(/sites\/all\/themes\//))
                    return;
            }
            
            //item = item.replace(/http:\/\/.*/i, '');
            $('#csidebar .stylelist').append('<option value="' + item + '">' + title + '</option>');
        }
        if(this.sList.length == 0){
            var self = this;
            
            //add links
            $('link').each(function(){
                if(this.type == 'text/css' || this.rel == 'stylesheet'){
                    addItem(this.href);
                    self.sList.push(this.href);
                }
                
            });
            //add styles
            $('style').each(function(){
                //every instance of '@import' is a stylesheet to replace
                //other styles are ignored
                var styles = $(this).html().match(/@import url\(['"](.*)['"]\)/gi);
                for(var s in styles){
                    var item = styles[s].substring(13, styles[s].length - 2);
                    addItem(item);
                    self.sList.push(item);
                }
            });
        }
        else{
            for(var i = 0; i < this.sList.length; i++)
                addItem(this.sList[i]);
        }
    },
    saveCSS: function(){
      $('#csidebar .save').html('...');
        var css = editor.getText();
        var href = this.curStyle;
        //post css up
        $.ajax({
            type: 'POST',
            url: Drupal.settings.basePath + 'css/save',
            data: {
                css: css,
                href: this.curStyle
            },
            dataType: "json",
            success: function(data){
                if(data.result == 'success'){
                    $('#csidebar .save').html('Save');
                }
                else{
                    alert(data.result);
                    $('#csidebar .save').html('Save');
                }
            }
        });
    },
    selectStyle: function(href, complete){
        //update selected
        this.curStyle = href;
        if(this.styleObjs[href] == null){
            this.styleObjs[href] = new style(href, complete);
        }
        else{
            complete(this.styleObjs[href].getStyle());
        }
    }
};

//sidebar ui setup
window.sidebar = {
    html: "<div id='csidebar'></div>",
    init: function(){
        //edit button
        var edittab = $("<div id='csstab'><div class='box'>Edit CSS</div></div>");
        //add tab to open
        $(document.body).append(edittab);
        
        var self = this;
        
        edittab.click(function(){
            self.show();    
            edittab.css('display', 'none');
            styler.init();
        });
        
        if(Drupal.settings.live_css.autoload){
            edittab.click();
        }
    },
    show: function(){
        
        $(document.body).parent().append(this.html);
        var self = this;
        
        //disable typical scrollbars
        $(document.body).parent().css('overflow', 'hidden');
        
        $(document.body).css({
            position: 'relative',
            overflow: 'scroll',
            'min-width': 0
        });
        
        //hide admin menu d7
        if($('#toolbar').length > 0 && Drupal.settings.live_css.hideadmin == 1){
            $('#toolbar').css('display', 'none');
            $(document.body).removeClass('toolbar');
            $(document.body).css('padding-top', '');
        }
        
        var size = function(){
            h = $(window).height();
            w = $(window).width();
            
            //set the body size
            $(document.body).css('width', w - 500);
            $(document.body).css('height', h - parseInt($(document.body).css('paddingTop')) - parseInt($(document.body).css('marginTop')));
            
            //set the sidebar size
            $('#csidebar').css('height', h);
            
            //editor size
            editor.height(h);
        };
        $(window).resize(size);
        size();
    },
    remove: function(){
        $('#csidebar').remove();
        $(document.body).parent().css('overflow', 'auto');
        $(document.body).css({
            position: 'static',
            overflow: 'visible',
            width: 'auto',
            height: 'auto'
        });
        
        $(window).unbind('resize');
        
       $('#csstab').css('display', 'block');
       
        //show admin menu d6
        if($('#toolbar').length > 0){
            $('#toolbar').css('display', 'block');
            $(document.body).addClass('toolbar');
            $(document.body).css('padding-top', '');
        }
    }
};

//class for generating stylesheet hooks from a style url
window.style = function(href, complete){
    this.href = href;
    var self = this;
    
    //given an href, attach the style and return the getter and setter
    
    //full uri conversion for css asset loading
    this.fullUri = function(css){
        var href = self.href;
        var parts = href.split('/');
        var baseURI = [];
        var dots = '';
        while(parts.pop()){
            baseURI.push(parts.join('/'));
            css = css.replace(new RegExp('url\\(' + dots, 'gi'), 'url(' + parts.join('/') + '/');
            dots += '../';
        }
        return css;
    }
    
    //short uri backwards conversion for css asset loading
    this.shortUri = function(css){
        var href = self.href;
        var parts = href.split('/');
        var baseURI = [];
        var dots = '';
        while(parts.pop()){
            baseURI.push(parts.join('/'));
            css = css.replace(new RegExp('url\\(' + parts.join('/') + '/', 'gi'), 'url(' + dots);
            dots += '../';
        }
        return css;
    }
    
    //check the links and styles
    $('link, style').each(function(){
        var element = this;
        
        //for link elements
        if(this.href == href){
            //insert this new stylesheet after the link element
            self.link = $("<style type='text/css'/>");
            self.link.type = "text/css";
            $(this).after(self.link);
            
            //load the stylesheet data and insert it into the new style element
            $.get(href, function(data){
                self.setStyle(data);
                complete(data);
            });
            
            self.setStyle = function(css){
                self.link.html(self.fullUri(css));
            };
            self.getStyle = function(){
                return self.shortUri(self.link.html());
            };
            
            //leave the loop
            return false;
        }
        
        //for style elements
        var html = $(this).html();
        if(html.indexOf(href) >= 0){
          
            self.setStyle = function(css){
                var href = self.href;
                href = href.replace(/\./g, '\\.');
                href = href.replace(/\//g, '\\/');
                href = href.replace(/\?/g, '\\?');
                var exp = new RegExp("\\/\\*\\+" + href + "\\*\\/(.|\\n|\\r)*\\/\\*\\-" + href + "\\*\\/");
                
                $(element).html($(element).html().replace(exp, '/*+' + self.href + '*/' + self.fullUri(css) + '/*-' + self.href + '*/'));
            };
            
            self.getStyle = function(){
                var href = self.href;
                href = href.replace(/\./g, '\\.');
                href = href.replace(/\//g, '\\/');
                href = href.replace(/\?/g, '\\?');
                var exp = new RegExp("\\/\\*\\+" + href + "\\*\\/(.|\\n|\\r)*\\/\\*\\-" + href + "\\*\\/");
                
                var style = $(element).html().match(exp)[0];
                return self.shortUri(style.substring(self.href.length + 5, style.length - self.href.length - 5));
            };
          
            if(html.indexOf('@import') >= 0){
                var html = $(this).html();
                
                //extract the full import list
                var files = html.match(/@import url\(['"]((.|\n|\r)*?)['"]\);/gi);
                
                //replace the @import codes with comment markers
                $(this).html(html.replace(/@import url\(['"]((.|\n|\r)*?)['"]\);/gi, '/*+$1*//*-$1*/\n\r'));
                
                //download all the import files for replacement into the markers
                $(files).each(function(){
                    var filename = this.substring(13, this.length - 3);
                    $.get(filename, function(data){
                        var href = filename;
                        href = href.replace(/\./g, '\\.');
                        href = href.replace(/\//g, '\\/');
                        href = href.replace(/\?/g, '\\?');
                        
                        var exp = new RegExp("\\/\\*\\+" + href + "\\*\\/(.|\\n|\\r)*\\/\\*\\-" + href + "\\*\\/");
                        $(element).html($(element).html().replace(exp, '/*+' + filename + '*/' + data + '/*-' + filename + '*/'));
                        
                        //run the complete function when we replace the one we wanted
                        if(filename == self.href){
                          complete(data);
                        }
                    });
                });
            }
            else{
              //styles already loaded
              complete(self.getStyle());
            }
            
            return false;
        }
        
        return true;
    });
  
    
};

//bespin editor wrapper class
window.editor = {
    html: "<div id='cedit'></div>",
    editor: null,
    change: null,
    init: function(o){
        $('#csidebar').append(this.html);
        
        $('#cedit').css('height', $(window).height() - ($('#cedit').offset().top - $('#csidebar').offset().top));
        
        var self = this;
        
        $('#cedit').html(o.css);
        
        var editor = ace.edit('cedit');
        var CSSMode = require("ace/mode/css").Mode;
        if(Drupal.settings.live_css.theme)
            editor.setTheme("ace/theme/" + Drupal.settings.live_css.theme);
        editor.getSession().setMode(new CSSMode());
        
        editor.setShowPrintMargin(false);
        editor.setHighlightActiveLine(false);
        editor.getSession().setTabSize(Drupal.settings.live_css.tabsize);
        editor.getSession().setUseSoftTabs(Drupal.settings.live_css.softtabs);
        $('#cedit').css('font-size', Drupal.settings.live_css.fontsize);
        
        
        //add keyboard shortcuts
        var canon = require('pilot/canon');
        // Fake-Save, works from the editor and the command line.
        canon.addCommand({
          name: "save",
          bindKey: {
            win: "Ctrl-S",
            mac: "Command-S",
            sender: "editor|cli"
          },
          exec: function() {
            styler.saveCSS();
          }
        });
        
        //add the change event
        if(o.change)
            editor.getSession().on('change', o.change);
        
        this.editor = editor;
        
        //by setting the width of the second canvas element to the window width, we avoid the flickering when resizing
        //$('#cedit canvas')[1].width = $(window).width();
        //$('#cedit canvas')[1].height = $(window).height();
    },
    onChange: function(changeFunc){
        editor.getSession().on('change', changeFunc);
    },
    width: function(sWidth){
        //set editor width
    },
    height: function(sHeight){
        $('#cedit').css('height', sHeight - 65);
        //if(this.editor)
        //    this.editor.dimensionsChanged();
    },
    getText: function(){
        return this.editor.getSession().getValue();
    },
    setText: function(css){
        this.editor.getSession().setValue(css);
        //set the line number
        var self = this;
        setTimeout(function(){
            self.editor.gotoLine(1);
        }, 100);
    }
};

$(document).ready(function(){
    sidebar.init();
});

})(jQuery);
