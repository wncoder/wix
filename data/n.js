var fs = require('fs');
console.log('Up.');

function read(callback, callback2, callback3) {
	fs.readFile('news.json', function (err, data) {
		if (err) throw err;

		console.log("Read.");
		var news = JSON.parse(data);
		callback(news, callback2, callback3);
	});
}

function process(news, callback, callback2) {
	for (var i = 0; i < news.length; i++) {
		var item = news[i];
		prettify(item.title, new Date(item.date), item.content, callback, callback2);
	}
}

function prettify(title, date, content, callback, callback2) {
	var yyyymmdd = date.getFullYear() + '-' + pad(date.getMonth() + 1, '00') + '-' + pad(date.getDate())
	var filename = 'src\\documents\\news\\' + yyyymmdd + '-' + slugify(title) + '.html.md';
	callback(filename, title, date, yyyymmdd, content, callback2);
}

function markdown(name, title, date, yyyymmdd, content, callback) {
	var md = '---\r\n';
	md += 'title: ' + title + '\r\n';
	md += 'date: ' + yyyymmdd + ' GMT-0800\r\n';
	md += '---\r\n';
	md += content + '\r\n';

	callback(name, md);
}

function write(filename, md) {
	fs.writeFile(filename, md, function (err) {
		console.log('Wrote: ' + filename);
		console.log(md);		
	});
}

function pad(number, padding) {
	zeros = typeof padding !== 'undefined' ? padding : '00';
	return ('0' + number).slice(-zeros.length);
}

function slugify(value) {
	return value.replace(/[^a-zA-Z0-9]/g, '-').replace(/-+$/, '').replace(/--+/g, '-');
}

read(process, markdown, write);

console.log('Down.');
// npm install -g git://github.com/perpetual-motion/lax-typescript.git