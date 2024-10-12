$(document).ready(function(){
  
  $('[data-behavior="file-settings"]').click(function(){
    let had = ($(this).parent().find('.file-menu').hasClass('collapsed'));
    $('.file-menu').addClass('collapsed');
    if (had) 
      $(this).parent().find('.file-menu').removeClass('collapsed');
  });
  
  
  $('.file-menu li').click(function(){
    $(this).parent().addClass('collapsed');
  })
})