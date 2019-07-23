"use strict";
/*
 * 
*/
var cgiRequests = {
	"servisemode": {
		requestFrom: "/getloopparam.cgi",
		actionFrom: function(){getParam("servisemode") },
		regular:	/^([\w^\d]+)=(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})/,
		callback: function(p){p;},
		requestTo: "/setloopparam.cgi",
		actionTo: function(){}
	},
	"controlsensors": {
		requestFrom: "/getloopparam.cgi",
		actionFrom: function(){getParam("controlsensors") },
		regular:	/^([\w^\d]+)=(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})/,
		callback: function(p){p;},
		requestTo: "/setloopparam.cgi",
		actionTo: function(){}
	},
	
	"version": {
		requestFrom: "/getversion.cgi",
		actionFrom: function() {getVersion()}
	},
};

var pages = {
	"servisemode": {
		content: "/txt/control/servisemode.txt",
		hash: "#!/servisemode",
		elem: [],
		action: cgiRequests["servisemode"].actionFrom,
		save: cgiRequests["servisemode"].actionTo
	},
	"controlsensors": {
		content: "/txt/control/controlsensors.txt",
		hash: "#!/controlsensors",
		elem: [],
		action: cgiRequests["controlsensors"].actionFrom,
		save: cgiRequests["controlsensors"].actionTo
	},
}

var timeoutRstId = setTimeout(function(){document.location.replace("/")}, 300000);

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
	timeoutRstId = setTimeout(function(){document.location.replace("/")}, 300000);
	return request.responseText;
}  
//

function BoL() {
	cgiRequests["version"].actionFrom();
}

function link_menu(param) {
	var content = ajquery(pages[param].content);
	if(content) {
		var pbdy = document.getElementById('content');
		pbdy.value = "";
		pbdy.innerHTML = content;
		document.location.hash = pages[param].hash;
		
		if (param == "controlsensors") {
			insertTableControl();
			configEventGuard();
		}
		
		pages[param].action();
	}
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

function getVersion() {
	var ps = ajquery (cgiRequests["version"].requestFrom);
	if (ps) {
		var str = document.getElementById('version') ;
		str.value = "";
		str.innerHTML = 'Версия ПО: ' + ps.slice(3);
	}
}

function getParam(elem) {
	var rq = ajquery (cgiRequests[elem].requestFrom);
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach( 
			function(item) {
				var res = cgiRequests[elem].regular.exec(item);
				if (res) {
					var el = document.getElementById("id_" + res[1]);
					cgiRequests[elem].callback(el);
					el.value = res[2];
				}
			}
		);
	}
}

/**
 * постановка на охрану / снятие с охраны
 */
var stateSensorsList = [
	"Снят с охраны",
	"На охране",
	"Невзятие",
	"Задержка взятия",
	"Задержка снятия"
	];

const btnNameList = ["Поставить на охрану", "Снять с охраны"];

function insertTableControl () {
	var a = document.getElementById('id_ctrl_guard');
	a.innerHTML = "";
	
	var str = "";

	for (let ix = 1; ix <= 6; ++ix) {
		str +=' \
			<div class="ctrl_tbl_string cntrl_tbl_brd_bot">\
			<div class="width50 border1px ctrl_div_elem_height"><label class="width95 center-please"/>Шлейф сигнализации №'+ix+'</label></div>\
			<div class="width25"><input id="id_stateguard'+ix+'" class="width95 cntrl_align ctrl_elem ctrl_off_guard" value="Снят с охраны" readonly/></div>\
			<div class="width25"><button id="id_setonguard'+ix+'" class="width95 cntrl_align ctrl_elem"/>Поставить на охрану</div>\
		</div>\
		';
	}
	
	a.innerHTML = str;
}

function configEventGuard () {
	
	for (let ix = 1; ix <= 6; ++ix) {
		let btn = document.getElementById("id_setonguard"+ix);
		//----------------------------------------------------------------------
		btn.onclick = function () {
			let btn = document.getElementById("id_setonguard"+ix);
			let lbl = document.getElementById("id_stateguard"+ix);
			
			if (btn.textContent == btnNameList[0]) {
				lbl.value = stateSensorsList[1];
				btn.textContent = btnNameList[1];
			}
			else {
				lbl.value = stateSensorsList[0];
				btn.textContent = btnNameList[0];
			}
		};
		//----------------------------------------------------------------------
	}
}
