const moment = require('moment');

const { tree, treeSync } = require('./build/Release/tree');
console.log('start |', moment().format('YYYY-MM-DD hh:mm:ss.SSS'));
tree('./', './output.json', () => {
    console.log('callback');
});
// const result = treeSync('./', './output.json');
console.log('end   |', moment().format('YYYY-MM-DD hh:mm:ss.SSS'));
