const moment = require('moment');

const { hello, world } = require('./build/Release/addon');
console.log('start |', moment().format('YYYY-MM-DD hh:mm:ss.SSS'));
const result = hello('C:\\msys64');
world();
console.log('end   |', moment().format('YYYY-MM-DD hh:mm:ss.SSS'));

console.log(result);
