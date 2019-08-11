const moment = require('moment');

const { tree, treeSync } = require('./build/Release/tree');
console.log('start |', moment().format('YYYY-MM-DD hh:mm:ss.SSS'));
const result = tree('C:\\Windows', '', () => {
    console.log('callback');
});
treeSync();
console.log('end   |', moment().format('YYYY-MM-DD hh:mm:ss.SSS'));

console.log(result);
