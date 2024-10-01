// upload a file
function fileUpload() {
    let files = [];
    let fileName = document.getElementById('file').value.split("\\").pop();
    files.push(fileName);
    console.log(files);
    // when the file is uploaded
    if (document.getElementById('file').value != "") {
        setTimeout(() => {
//           create list of files
            files.forEach(function(item) {
                item = item.toLowerCase();
                let list = document.createElement("li");
                list.innerText = item;
                 document.getElementById('list').appendChild(list);
               })
        }, 400);
    }
}