const addon = require(`../build/Release/tree`);
const json = addon.tree('.');
console.log(json);
