'use strict'
/*jshint esversion: 6 */
/* jshint browser: true */

var is_def_hw = 0;
var timeoutRstId = setTimeout( () => document.location.replace("/"), 300000);

function ajquery(sURL) {  
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
/*****************************************************************************/
function Context (rqF = '/', rqT = '/', content = '', hash = '', reg = null) {
	let self = this;

	this._data = {};
	this._requestTo = rqT;

	function request () {
		if (reg) {
			let rq = ajquery (rqF);
			rq  = rq.split('\n');
			rq.forEach ( (item) => {
				item = reg.exec(item);
				if (item) self._data[item[1]] = item[2];
			}); 
		}
	}
	request();

	this.updateData = () => request();
	this.getData = () => {
		if (Object.keys(self._data).length === 0) request();
		for (let key in self._data) {
			document.getElementById("id_" + key).value = self._data[key];
		}
	};

	this.getContent = () => ajquery (content);
	this.hash = () => hash;

	this.setData = (def) => {
		let str = '?';
		if (def == 1) {
			str += "default=";
		}
		else {
			for (let key in self._data) {
				let el = document.getElementById("id_" + key);
				str += key+"="+el.value+"&";
			}
		}
		return str;
	};
}

function Version (par) {
	Context.apply(this, arguments);
	let self = this;
	this.getData = () => self._data["fw="];
}


function CheckedHw (p) {
	Context.apply(this, arguments);
	let rq = ajquery (p);
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

function isValidIp(val) {
	var ipPattern = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
	var ipArray = val.match(ipPattern);
	if ((val=="")||(val=="0.0.0.0")||(val=="255.255.255.255")||(ipArray==null)||(val.length>15)) {return 0;}
	if (ipArray[4] == 255) return 0;
	for (var i = 1; i < 5; i++)
		if (ipArray[i] > 255) return 0;
	return 1;
}

function Ip (rqF = '/', rqT = '/', content = '', hash = '') {
	Context.apply(this, arguments);
	let self = this;

	function checkValidParam () {
		let flag = 0;
		for (let key in self._data) {
			let el = document.getElementById("id_" + key);
			if (isValidIp(el.value)) {
				flag++;
			}
			else {
				alert(key + 'адрес: ошибка!'); 
				el.focus(); 
				flag=0; 
			}
		}
		return flag;
	}

	this.setParam = (def) => {
		if ( checkValidParam() == 3) {
			let str = self.setData(def);
			str = str.replace(/\./g, "|");
			ajquery (self._requestTo + str.substring(0, str.length - 1));
		}
	};
}

function Login (rqF = '/', rqT = '/', content = '', hash = '') {
	Context.apply(this, arguments);
	let self = this;

	this.setParam = (def) => {
		var rq;
		if (def == 1) {
			rq = ajquery (self._requestTo+"?default=");
		}
		else {
			var str = "?";
			
			['login', 'password'].forEach( (item) => {
				var el = document.getElementById("id_" + item);
				var tmp = el.value.match(/^[a-zA-Z0-9]{4,17}$/);
				
				if (item === "password") {
					if (el.value.replace(/\s+/g, '').length == 0) {
						tmp = "";
					}
				}

				str += item+"="+tmp+"&";
			});
			
			rq = ajquery (self._requestTo + str.substring(0, str.length - 1));
			
			rq = rq.replace(/(\r\n|\n|\r)/gm,"");
			if (rq === "OK!") {
				var loc = document.location.hash;
				loc = loc.substr(3);
				link_menu(loc);
			}
			else {
				var result = document.getElementById('result');
				result.className = 'result fail';
				result.innerHTML = 'ОШИБКА: Неправильный логин или пароль. Возможно у вас выбрана другая раскладка клавиатуры или нажата клавиша "Caps Lock". Проверьте правильность введённых данных и повторите попытку.';
				result.style.display = 'block';
			}
		}
	};
}

function Hw (rqF = '/', rqT = '/', content = '', hash = '') {
	Context.apply(this, arguments);
	let self = this;

	function checkValidParam () {
		let flag = 0;
		for (let key in self._data) {
			let el = document.getElementById("id_" + key);

			if (el.value.match(/^([0-9A-Fa-f]{2}(:[0-9A-Fa-f]{2}){5})$/)) {
				flag++;
			}
			else {
				alert('hw адрес: ошибка!'); 
				el.focus(); 
				flag=0; 
			}
		}
		return flag;
	}

	this.setParam = (def) => {
		if ( checkValidParam() == 1) {
			let str = self.setData(def);
			str = str.replace(/\:/g, "|");
			rebootFromPages(self._requestTo, str.substring(0, str.length));
		}
	};
}

function Converter  (rqF = '/', rqT = '/', content = '', hash = '') {
	Context.apply(this, arguments);
	let self = this;

	this.setParam = (def) => {
		var str = "?";
		['mode', 'speed', 'hostport', 'hostip', 'port'].forEach( function(item) {
			var el = document.getElementById("id_" + item);	
			if (item === "hostip") {
				if (isValidIp (el.value)) {
					str += item+"="+el.value.replace(/\./g, "|")+"&";
				}
			}
			else {
				str += item+"="+ el.value+"&";		
			}
		});
		if (def) str = "?default=";
		var rq = ajquery (self._requestTo + str);
				
		rq = rq.replace(/(\r\n|\n|\r)/gm,"");
		if (rq == "OK!") {
			alert("param is set OK!");
		}
		else {
			alert("param is not set!");
		}
	};
}
/*****************************************************************************/
var version = new Version ("/getversion.cgi", null, null, null, /^(fw=)(.*)/);
var checked_hw = new CheckedHw ("/checkhw.cgi");

var ip = new Ip ("/getipparam.cgi", "/setipparam.cgi", "/txt/admin/ip.txt", "#!/ip", /^([\w^\d]+)=(\d{1,3}(.\d{1,3}){3})/);
var login = new Login ("/getlogin.cgi", "/setlogin.cgi", "/txt/admin/authorization.txt", "#!/login", /^([\w^\d]+)=(\w{4,17})/);
var hw = new Hw ("/gethwparam.cgi", "/sethwparam.cgi", "/txt/admin/hw.txt", "#!/hw", /^([\w^\d]+)=([0-9A-Fa-f]{2}(:[0-9A-Fa-f]{2}){5})/);
var converter = new Converter ("/getconverter.cgi", "/setconverter.cgi", "/txt/admin/converter.txt", "#!/converter", /^([\w^\d]+)=((\d{1,3}(.\d{1,3}){3})|(\w?\d+))/);
var system = new Context (null, null, "txt/admin/system.txt", "#!/system", null);

function BoL () {
	let str = document.getElementById('version') ;
	str.value = "";
	str.innerHTML = 'Версия ПО: ' + version.getData();

	let ref = document.getElementById("menu_2");
	if (is_def_hw == 1) {
		document.getElementById("mac_state").style.display = 'table';
		ref.style.background = '#FF4D4D';
	}
	else {
		document.getElementById("mac_state").style.display = 'none';
		ref.style.background = '#49E';
	}
}

function chp(event){
	if (event.keyCode==13) {
		var form = document.querySelector("form");
		if (form) {
			if(form.name === 'form_ip'){checkAuth();}
			if(form.name === 'form_login'){checkAuth();}
			if(form.name === 'form_auth'){checkAuth();}
			if(form.name === 'form_hw'){checkAuth();}
		}
	}
}

function setParam(param) {
	window[param].setParam(0);
}

function setDefParam(param) {
	window[param].setParam(1);
}

function link_menu (param) {
	let content = param.getContent();
	if (content) {
		var pbdy = document.getElementById('content');
		pbdy.value = "";
		pbdy.innerHTML = content;
		document.location.hash = param.hash();
		param.getData();
	}
	
	if (param == "converter") {
		let id = document.getElementById('id_out1');
		id.onchange = () =>	handlerOtherState("id_mode", "id_out1");

		id = document.getElementById('id_mode');
		id.onchange = () =>	handlerOtherState("id_mode", "id_out1", callbackConverter);
	}
}

function checkAuth() {
	var textPattern=/^[a-zA-Z0-9]{4,17}$/;
	var str = "?";
	
	['login', 'password'].forEach( (item) => {
		let el = document.getElementById("id_" + item);
		let tmp = el.value.match(textPattern);
		
		str += item+"="+tmp+"&";
	});
	
	let rq = ajquery ("/checkauthorizations.cgi" + str.substring(0, str.length - 1));
	rq = rq.replace(/(\r\n|\n|\r)/gm,"");
	if (rq === "OK!") {
		let loc = document.location.hash;
		loc = loc.substr(3);
		link_menu(window[loc]);
	}
	else {
		let result = document.getElementById('result');
		result.className = 'result fail';
		result.innerHTML = 'ОШИБКА: Неправильный логин или пароль. Возможно у вас выбрана другая раскладка клавиатуры или нажата клавиша "Caps Lock". Проверьте правильность введённых данных и повторите попытку.';
		result.style.display = 'block';
	}
}

function goToBoot () {
	rebootFromPages("/gotoboot.cgi", null);
}

function rebootFromPages (item, data) {
	let content;
	if (data == null) content = ajquery(item);
	else content = ajquery(item+data);

	if (content) {
		content = content.replace(/(\r\n|\n|\r)/gm,"");
		if (content == "OK!") {
			rebootDev();
		}
	}
}

function rebootDev () {
	var rebootPage = ajquery("/txt/reboot.txt");
	if (rebootPage) {
		var pbdy = document.getElementById('content');

		pbdy.innerHTML = rebootPage;

		var pnet = ajquery('/rstip.cgi');
		if (pnet) {
			var mass = [];
			mass = pnet.split('\n');
			var reg = /^([\w^\d]+)=(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})/;

			var ip = document.getElementById('ipNew');
			var tmp = mass[0].match(reg);
			ip.href = 'http://'+tmp[2]+'/';
			ip.innerHTML = 'http://'+tmp[2]+'/';

			ip = document.getElementById('ipDef');
			tmp = mass[1].match(reg);
			ip.href = 'http://'+tmp[2]+'/';
			ip.innerHTML = 'http://'+tmp[2]+'/';
			
			var rebootCmd = ajquery('/reboot.cgi');
			
			if (rebootCmd) {
				rebootCmd = rebootCmd.replace(/(\r\n|\n|\r)/gm,"");
				if (rebootCmd === "OK!") {
					tmp = mass[0].match(reg);
					var cmd = 'location.href="http://'+tmp[2]+'/index.html"';
					setTimeout(cmd, 5000);
				}
			}
		}
	}
}

function applyParam() {
	let rq = ajquery ("/applyparam.cgi");
	rq = rq.replace(/(\r\n|\n|\r)/gm,"");
}

function handlerOtherState(selId, checkId, funct, ix) {
	selId = document.getElementById(selId);
	checkId = document.getElementById(checkId);
	
	if (ix === undefined)
		ix = null;
	if (funct === undefined)
		funct = null;
	else {
		funct(selId, checkId, ix);
	}
	
	if (selId.selectedIndex == 0)
		checkId.checked = false;
}

function callbackConverter (sel, check, ix) {
	if (sel.selectedIndex != 0)
		check.checked = true;	
}
