var flipTime = 1000;
var wrapper = document.querySelector(".clock__number-wrapper");

setInterval(function () {
  flipNumber();
}, flipTime);

function flipNumber () {
  wrapper.classList.add("clock__number-wrapper--open");
  setTimeout(function () {
    increment(".js-increment-before");
    wrapper.classList.remove("clock__number-wrapper--open");
    increment(".js-increment-after");
  }, flipTime / 2);
}

function increment (selector) {
  Array.from(document.querySelectorAll(selector)).forEach(function (el) {
    el.innerHTML = ((parseInt(el.innerHTML) + 1) % 10);
  });
}