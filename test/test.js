const addon = require(`../build/Release/tree`);

const root = process.platform === 'win32' ? 'C:/Program Files' : '/';
const outputSync = './output.sync.json';
const outputAsync = './output.async.json';

const main = async () => {
    const startSync = new Date().getTime();
    console.log('sync start');
    const json = addon.treeSync(root, outputSync);
    const endSync = new Date().getTime();
    console.log('sync end', `${endSync - startSync}[ms]`);

    const startAsync = new Date().getTime();
    console.log('async start');
    addon.tree(root, outputAsync, (result) => {
        const endAsync = new Date().getTime();
        console.log('async', `${endAsync - startAsync}[ms]`);
    });
    console.log('end');
};

main();
