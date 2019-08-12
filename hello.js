const moment = require('moment');

const { tree, treeSync } = require('./build/Release/tree');

const sleep = (time) => {
    return new Promise((resolve) => {
        setTimeout(() => {
            resolve();
        }, time);
    });
};

const main = async () => {

    console.log('start |', moment().format('YYYY-MM-DD hh:mm:ss.SSS'));
    tree('C:\\msys64', './output.json', (result) => {
        console.log('callback', result);
    });
    // const result = treeSync('./', './output.json');
    console.log('end   |', moment().format('YYYY-MM-DD hh:mm:ss.SSS'));

    do {
        console.log('sleep |', moment().format('YYYY-MM-DD hh:mm:ss.SSS'));
        await sleep(1000);
    } while (true);
};

main();
