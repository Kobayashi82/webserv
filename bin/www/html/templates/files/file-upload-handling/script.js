// This function just works and can be used for many file types.
// Don't try to reinvent this
function readURL(input,callback) {
  if (input.files && input.files[0]) {
    var reader = new FileReader();
    reader.onload = callback;
    reader.readAsDataURL(input.files[0]);
  }
}






// Create a selector for an input and then do whatever you want using the callback function.
$(".imagepicker-replace input").on("change",function() {
  // store the current "this" into a variable
  var imagepicker = this;
  readURL(this,function(e) {
    // "this" will be different in the callback function
    $(imagepicker).parent()
      .addClass("picked")
      .css({"background-image":"url("+e.target.result+")"});
  });
});



// This example will add a new thumbnail each time
$(".imagepicker-add input").on("change",function() {
  var imagepicker = this;
  readURL(this,function(e) {
    $(imagepicker).parent().before(
      "<div class='thumbnail' style='background-image:url("+e.target.result+")'></div>"
    )
  });
});