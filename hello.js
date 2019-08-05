const fs = require('fs');

const addon = require('./build/Release/addon');
const result = addon.hello('C:\\msys64\\home\\todo\\github\\tree\\parent');
console.log('addon.hello result');
console.log(result);

// const json = JSON.stringify(result);

// // fs.writeFile("output.json", json, 'utf8', err => {
// //   if (err) {
// //     console.log("An error occured while writing JSON Object to File.");
// //     return console.log(err);
// //   }

// //   console.log("JSON file has been saved.");
// // });
