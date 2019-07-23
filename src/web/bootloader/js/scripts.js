//
var cgiRequests = {
	"version": {
		requestFrom: "/getversion.cgi",
		actionFrom: function() {getVersion()}
	},
	"checkauth": {
		requestFrom: "/checkauthorizations.cgi",
		actionFrom: function() {checkAuth()}
	},
	"uploadfile": {
		requestFrom: "/uploadfile.cgi",
		actionFrom: function() {uploadFile()}
	}
}

var pages = {
	"login": {
		hash: "#!/login",
		elem: ['login', 'password']
	},
	"choice": {
		content: "/txt/choice.txt",
		hash: "#!/choice",
	},
	"upload": {
		content: "/txt/upload.txt",
		hash: "#!/upload",
	},
	"uploaddone": {
		content: "/txt/uploaddone.txt",
		hash: "#!/uploaddone",
	},
	"reset": {
		content: "/txt/reset.txt",
		hash: "#!/reset",
	},
	"uploadfalse": {
		content: "/txt/uploadfalse.txt",
		hash: "#!/false",
	},
	"uncorfirm": {
		content: "/txt/uncorfirm.txt",
		hash: "#!/unjcor",
	}
}

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
	return request.responseText;}  
	
function ajqueryGETasync(sURL, f_ok, f_err, timeout){  
	request=null;
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
	request.open('GET', sURL, true);
	request.timeout = timeout; // 
	request.onload = f_ok;
	request.ontimeout = f_err;
	request.send(null);
	return request.responseText;}
//

function BoL() {
	cgiRequests["version"].actionFrom();
}

function chp(event){
	if(event.keyCode==13){
		var form = document.querySelector("form");
		if(form) {
			if(form.name === 'form_ip'){checkAuth();}
			if(form.name === 'form_login'){checkAuth();}
			if(form.name === 'form_auth'){checkAuth();}
			if(form.name === 'form_hw'){checkAuth();}
		}
	}
}

function link_menu(param) {
	var content = ajquery(pages[param].content);
	if(content) {
		var pbdy = document.getElementById('content');
		pbdy.value = "";
		pbdy.innerHTML = content;
		document.location.hash = pages[param].hash;

		if (param == 'reset') ajquery('/reboot.cgi');

	}
}

function getVersion() {
	var ps = ajquery (cgiRequests["version"].requestFrom);
	if (ps) {
		var str = document.getElementById('version') ;
		str.value = "";
		str.innerHTML = 'Версия ПО: ' + ps;
	}
}

function checkAuth() {
	var textPattern=/^[a-zA-Z0-9]{4,17}$/;
	var str = "?";
	pages["login"].elem.forEach( function(item) {
		var el = document.getElementById("id_" + item);
		var tmp = el.value.match(textPattern);
		
		str += item+"="+tmp+"&";
	});
	rq = ajquery (cgiRequests["checkauth"].requestFrom + str.substring(0, str.length - 1));
	rq = rq.replace(/(\r\n|\n|\r)/gm,"")
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

var mass = new Uint8Array();
var crc = new Uint16Array(1);
var length;

function loadFile() {
	var control = document.getElementById('datafile');
	var reader = new FileReader();
	var files = control.files;
	var len = files.length;
	var i = 0;
	crc[0] = 0xFFFF;
	reader.onload = function(event) {
		var contents = event.target.result;
		var param = new Uint8Array(contents);
		mass = param;
		length = param.length;

		var but = document.getElementById('loadF');
		but.disabled = false;
		var elem = document.getElementById('myProgress');
		elem.style.display = 'table';
	}; 
	reader.readAsArrayBuffer(files[0]);
}

var ix = 0;
var part = 0;
var content;

function progressBarHandler (x) {
	if (x) {
		isSuccess = 1;
		countErr = 0;
		++part;
		ix += 512;
		var lenPer = (ix * 100) / length;
		var elem = document.getElementById('myBar');
		elem.style.width = Math.round(lenPer) + '%';
		elem.innerHTML = Math.round(lenPer) * 1  + '%';
	}
	else {
		isSuccess = 0;
		++countErr;
	}
}

function recvfirmInfo() {
	content = request.responseText;
	content = content.replace(/(\r\n|\n|\r)|\0/gm,"");
	if (content === "KEY IS MATCH") {
		ajqueryGETasync("/isFirmNew.cgi", isFirmNew, TimeoutHandler, 10000);
		progressBarHandler(1);
	}
	else {
		isProgBarInc = 0;
		link_menu("uncorfirm");//document.location.replace("/uploadFalse.cgi");
	}
}

function eraseFirmBlock() {
	setTimeout(function(){uploadFile()}, 1);
}

function isFirmNew() {
	content = request.responseText;
	content = content.replace(/(\r\n|\n|\r)|\0/gm,"");
	var isProgBarInc = 0;
	var mass=new Array();
	mass = content.split('\:\:');
	var lastPart = Math.floor((length-512)/512 + ((length-512) % 512 ? 1 : 0));
	if (mass[0] === "FIRM IS MATCH") {
		if (parseInt(mass[1], 10) < lastPart) {
			var r=confirm("Обнаружена незавершенная загрузка прошивки.\r\nПродолжить загрузку с прерванного места?\r\nПри нажатии CANCEL загрузка начнется с начала.");
			if (r==true) {
				part = parseInt(mass[1], 10);
				ix = parseInt(mass[1], 10) * 512;
				setTimeout(function(){uploadFile()}, 1);
			}
			else {
				part = 1;
				ix = 512;
				ajqueryGETasync("/eraseFirmBlock.cgi", eraseFirmBlock, TimeoutHandler, 10000);
			}
		}
		else if (parseInt(mass[1], 10) == lastPart) {
			var r=confirm("Устройство уже содержит данную прошивку.\r\nВсе равно желаете загрузить прошивку?\r\n");
			if (r==true) {
				part = 1;
				ix = 512;
				ajqueryGETasync("/eraseFirmBlock.cgi", eraseFirmBlock, TimeoutHandler, 10000);
			}
			else {
				part = parseInt(mass[1], 10);
				ix = length;
				setTimeout(function(){uploadFile()}, 1);
			}
		}
		else {
			part = 1;
			ix = 512;
			ajqueryGETasync("/eraseFirmBlock.cgi", eraseFirmBlock, TimeoutHandler, 10000);
		}
	}
	else if (mass[0] === "FIRM IS NEW") {
		part = 1;
		ix = 512;
		ajqueryGETasync("/eraseFirmBlock.cgi", eraseFirmBlock, TimeoutHandler, 10000);
	}
}

function ContentHandler() {
	var isProgBarInc = 0;
	content = request.responseText;
	if (content) {
		content = content.replace(/(\r\n|\n|\r)|\0/gm,"");
		if(content === "UPLOAD OK")	{
			progressBarHandler(1);
		} 
	}
	setTimeout(function(){uploadFile()}, 1); 
}

var countErr = 0;
function TimeoutHandler() {
	if(countErr < 3) {
		setTimeout(function(){uploadFile()}, 1);
	} else {
		document.location.replace("/uploadFalse.cgi");
	}
}

function uploadFile() {
	var tmp = new Uint8Array(512);
	var isSuccess = 1;
	var jx;
	var idx = ix;
	var countErr = 0;
	var but = document.getElementById('loadF').disabled = true;

	if (idx < length && countErr < 10)	{
		if (isSuccess == 1) {
			for (jx = 0; jx != 512 && idx < length; ++jx, ++idx)
				tmp[jx] = mass[idx];
		}
	}
		
	var t_len = jx;
	var i = 0;
	var crc1 = new Uint16Array(1);
	crc1[0] = 0xFFFF;

	while (t_len--)	{
		var m = tmp[i];
		crc1[0] ^= m << 8;
		i++;
		for(var j = 0; j < 8; j++)
			crc1[0] = crc1[0] & 0x8000 ? (crc1[0] << 1) ^ 0x1021 : crc1[0] << 1;
	}

	var str = "?numberPart=" + part;
	str += "&length=" + jx;
	str += "&crc=" + crc1[0];
	str += "&uploadPart=";

	for (var j = 0; j < jx; ++j) {
		var tstr;
		if (tmp[j] < 16) {
			tstr = 0;
			tstr += tmp[j].toString(16);
		} else {
			tstr = tmp[j].toString(16);
		}
		str += tstr;
	}
				
	if(part == 0) {
		ajqueryGETasync("/firmInfo.cgi"+str, recvfirmInfo, TimeoutHandler, 10000);
	}
	else {
		ajqueryGETasync("/uploadFile.cgi"+str, ContentHandler, TimeoutHandler, 3000);	
	}
	
	if (ix >= length) {
		link_menu('uploaddone');
	} 
}

function getCurrentFirm () {
	link_menu('uploaddone');
	ajquery('/getcurrentfirm.cgi');
}