"use strict"
/*jshint esversion: 6 */ 

function Context (rq = '/', content = '', hash = '') {
	this._request = () => ajquery (rq);
	this._content = () => ajquery (content);
	this._hash = hash;
}

function Version (par) {
	Context.apply(this, arguments);

	var vers = "";
	let ps = this._request();
	if (ps) {
		vers = ps.slice(3);
	}

	this.getVersion = () => vers;
}

function HW (par) {
	Context.apply(this, arguments);

	let rq = this._request();
	if (rq) {
		let reg=/^([\w^\d]+)=([0-1]{1})/;
		let mass = [];
		mass = rq.split('\n');

		mass.forEach( (item) => {
			let res = reg.exec(item);
			if (res) {
				if (res[1] === "is_def") {
					let el = document.getElementById("id_hw");
					is_def_hw = res[2];
				}
			}
		});
	}
}
/*******************************************************************************/
var timeoutRstId = setTimeout( () => document.location.replace("/"), 300000);
function ajquery(sURL){  
		var request=null;
		if (!request) try {
			request = new ActiveXObject('Msxml2.XMLHTTP');
			} catch (e){}
		if (!request) try {
			request = new ActiveXObject('Microsoft.XMLHTTP');
			} catch (e){}
		if (!request) try {
			request = new XMLHttpRequest();
			} catch (e){}
		if (!request)
			return "";
	request.open('GET', sURL, false);
	request.send(null);
	clearTimeout(timeoutRstId);
	timeoutRstId = setTimeout( () => document.location.replace("/"), 300000);
	return request.responseText;
}

var is_def_hw;
var version = new Version ("/getversion.cgi");
var hw = new HW ("/checkhw.cgi");

function BoL() {
	let str = document.getElementById('version') ;
	str.value = "";
	str.innerHTML = 'Версия ПО: ' + version.getVersion();

	var mac = document.getElementById("mac_state");
	var ref = document.getElementById("menu_0");
	if (is_def_hw == 1) {
		mac.style.display = 'table';
		if (ref) ref.style.background = '#FF4D4D';
	}
	else {
		mac.style.display = 'none';
		if (ref) ref.style.background = '#49E';
	}
}
