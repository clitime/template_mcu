/* jshint browser:true*/
'use strict';


/*

*/

if (document.all && document.documentMode && 8 === document.documentMode) {
    alert('IE11 не совместим!');
}

if (Object.hasOwnProperty.call(window, "ActiveXObject") && !window.ActiveXObject) {
    alert("IE11 не совместим!");// is IE11
}

(function ttt() {
	let IE='\v'=='v';
	if (IE)
		alert("Не совместим!");
})();

const Q_ALARM_LOOP = 8;
const Q_OUTPUT = 6;
const Q_INPUT = 4;

var cgiRequests = {
	"configloop": {
		requestFrom: "/getloopparam.cgi",
		actionFrom: function () {getParam("configloop") },
		regular:	/^([\w^\d]+)=(\w?\d+)/,
		callback: function (p) {
			for (let i = 0; i != Q_ALARM_LOOP; ++i) {
				if (p.id == "id_mode"+(i)) {
					p.onchange();
					document.getElementById("id_loop"+(i)).click();

					loopObj[i].mode = p.value;
				}

				if (p.id == "id_check_param"+(i)) {
					for (let iy = 0; iy != 6; ++iy) {
						let ch = document.getElementById("id_check_param_"+(iy)+"_"+(i+1));
						ch.checked = false;
						if (p.value & CHECK_MASK_MASS[iy]) {
							if (iy == 5)
								ch.selectedIndex = 1;
							else
								ch.checked = true;
						}

						switch(iy) {
							case 0: case 1: case 2: case 3: case 4:
								if (ch.checked)
									loopObj[(i)].check_param = loopObj[(i)].check_param | CHECK_MASK_MASS[iy];
								else
									loopObj[(i)].check_param = loopObj[(i)].check_param & ~CHECK_MASK_MASS[iy];
								break;
							case 5:
								if (ch.selectedIndex == 1)
									loopObj[(i)].check_param = loopObj[(i)].check_param | CHECK_MASK_MASS[iy];
								else
									loopObj[(i)].check_param = loopObj[(i)].check_param & ~CHECK_MASK_MASS[iy];
								break;
						}
						loopObj[i].change = false;
					}
				}
			}
		},
		requestTo: "/setloopparam.cgi",
		actionTo: function(){}
	},
	"controlsensors": {
		requestFrom: "/getcontrolsensors.cgi",
		actionFrom: function(){getParam("controlsensors") },
		regular:	/^([\w^\d]+)=(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})/,
		callback: function(p){p;},
		requestTo: "/setcontrolsensors.cgi",
		actionTo: function(){}
	},
	"configsensor": {
		requestFrom: "/getsensorparam.cgi",
		actionFrom: function(){getParam("configsensor") },
		regular:	/^([\w^\d]+)=(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})/,
		callback: function(p){p;},
		requestTo: "/setsensorparam.cgi",
		actionTo: function(){}
	},
	"configsection": {
		requestFrom: "/getsectionparam.cgi",
		actionFrom: function(){getParamSection("configsection") },
		regular:	/^([\w]+)_254_([\d]+)=(\d{1,2})/,
		callback: function(p){p;},
		requestTo: "/setsectionparam.cgi",
		actionTo: function(){}
	},
	"configoutput": {
		requestFrom: "/getoutputparam.cgi",
		actionFrom: function(){getParam("configoutput") },
		regular:	/^([\w^\d]+)=(\d{1,10})/, ///^([\w]+)([\d]+)=(\d{1,2})/,
		callback: function(p){
			/*

			*/
	var rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
	}

	for (let i = 0; i != Q_OUTPUT; ++i) {
		if (p.id == "id_source_sensors" + i) {
			var t = document.getElementById('id_tbl_celS' + (i+1)) ;
			t.value = "";

			var b1 = "";
			let sens_hide = document.getElementById ('id_source_sensors' + i);
			let sens_hide_h = document.getElementById ('id_source_sensors_h' + i);

			let index = 0x01;

			for (let ix = 0; ix != sensorsSectionArray.length; ++ix) {
				if (ix in sensorsSectionArray == false) {
					index <<= 1;
					continue;
				}

				if (ix >= Q_ALARM_LOOP) {
					mass.forEach (
						function(item) {
							var res = getSensorsParamc(item);
							if (res) {
								let iy = parseInt(res[1]) + Q_ALARM_LOOP;

								if (iy == ix) {
									if (ix > 31) {
										if (sens_hide_h.value & index) {
											b1 += '<div id="id_div';
											b1 += ix;
											b1 += '">	<label><b>'+nameDev[res[2]]+' № '+res[1];
											b1 += '</b></label>';
										}
									}
									else {
										if (sens_hide.value & index) {
											b1 += '<div id="id_div';
											b1 += ix;
											b1 += '">	<label><b>'+nameDev[res[2]]+' № '+res[1];
											b1 += '</b></label>';
										}
									}
								}
							}
						}
					);
				}
				else {
					if (sens_hide.value & index) {
						b1 += '<div id="id_div';
						b1 += ix;
						b1 += '">	<label><b>Шлейф № ';
						b1 += ix+1;
						b1 += '</b></label>';
					}
				}
				index <<= 1;
			}

			t.innerHTML = b1;
		}

		if (p.id == "id_source_states" + i) {
			var t = document.getElementById('id_tbl_cel' + (i+1)) ;
			t.value = "";

			var b1 = "";
			let ev_hide = document.getElementById ('id_source_states' + i)
			let index = 0x01;

			for (let ix = 0; ix != stateSensorsList.length; ++ix) {
				if (ev_hide.value & index) {
					b1 += '<div id="id_div';
					b1 += ix;
					b1 += '">	<label><b>';
					b1 += stateSensorsList[ix];
					b1 += '</b></label>';
				}
				index <<= 1;
			}

			t.innerHTML = b1;
		}
	}
		},
		requestTo: "/setoutputparam.cgi",
		actionTo: function(){}
	},
	"configkeyaccess": {
		requestFrom: "/getkeyaccessparam.cgi",
		actionFrom: function(){getParam("configkeyaccess") },
		regular:	/^([\w^\d]+)=(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})/,
		callback: function(p){p;},
		requestTo: "/setkeyaccessparam.cgi",
		actionTo: function(){}
	},
	"version": {
		requestFrom: "/getversion.cgi",
		actionFrom: function() {getVersion()}
	},
	"apply": {
		requestFrom: "/applyparam.cgi",
		actionFrom: function() {applyParam()}
	},
	"login": {
		requestFrom: "/getlogin.cgi",
		actionFrom: function(){getParam("login")},
		regular: /^([\w^\d]+)=(\w{4,17})/,
		callback: function(p){p;},
		requestTo: "/setlogin.cgi",
		actionTo: function(def){setLogin(def)},
	},
	"checkauth": {
		requestFrom: "/checkauthorizations.cgi",
		actionFrom: function() {checkAuth()}
	},
	"ctrlsensor": {
		requestFrom: "/getcontrolsensors.cgi",
		actionFrom: function() {getCtrlSensors()},
		requestTo: "/setcontrolsensors.cgi",
		actionTo: function() {setCtrlSensors()}
	},
	"loopvalue": {
		requestFrom: "/getloopvalue.cgi",
		regular:	/^(\w+)(\d)=((\d{1,6})|(\w+))/,
	},
	"loopstate": {
		requestFrom: "/getalarmloopstate.cgi",
		regular:	/^(\w+)_254_(\d)=(\d{1,6})/,
	},
	"loopguardstate": {
		requestFrom: "/getloopstate.cgi",
		regular:	/^(\w+)_(\d+)_(\d)=(\d{1,6})/,
	},
	"outputstate" : {
		requestFrom: "/getoutputstate.cgi",
		regular:	/^(\w+)_254_(\d)=(\d{1})/,
	},
	"inputstate" : {
		requestFrom: "/getinputstate.cgi",
		regular:	/^(\w+)_254_(\d)=(\d{1})/,
	},
};

var pages = {
	"configloop": {
		content: "/txt/loops/configloop.txt",
		hash: "#!/configloop",
		elem: ['check_param', 'time_delay_guard', 'delay_alarm', 'delay_recovery_alarm', 'range0', 'range1', 'range2', 'range3', 'range4'],
		action: cgiRequests["configloop"].actionFrom,
		save: cgiRequests["configloop"].actionTo
	},
	"controlsensors": {
		content: "/txt/loops/controlsensors.txt",
		hash: "#!/controlsensors",
		elem: [],
		action: cgiRequests["controlsensors"].actionFrom,
		save: cgiRequests["controlsensors"].actionTo
	},
	"configsensor": {
		content: "/txt/loops/configsensor.txt",
		hash: "#!/configsensor",
		elem: [],
		action: cgiRequests["configsensor"].actionFrom,
		save: cgiRequests["configsensor"].actionTo
	},
	"configsection": {
		content: "/txt/loops/configsection.txt",
		hash: "#!/configsection",
		elem: [],
		action: cgiRequests["configsection"].actionFrom,
		save: cgiRequests["configsection"].actionTo
	},
	"configoutput": {
		content: "/txt/loops/configoutput.txt",
		hash: "#!/configoutput",
		elem: [],
		action: cgiRequests["configoutput"].actionFrom,
		save: cgiRequests["configoutput"].actionTo
	},
	"configkeyaccess": {
		content: "/txt/loops/configkeyaccess.txt",
		hash: "#!/configkeyaccess",
		elem: [],
		action: cgiRequests["configkeyaccess"].actionFrom,
		save: cgiRequests["configkeyaccess"].actionTo
	},
	"login": {
		content: "/txt/admin/authorization.txt",
		hash: "#!/login",
		elem: ['login', 'password'],
		action: cgiRequests["login"].actionFrom,
		save: cgiRequests["login"].actionTo
	},
	"search":{
		content: "/txt/loops/searchSensors.txt",
		hash: "#!/search",
		elem: [],
		action: null,
		save: null
	},
	"searchR":{
		content: "/txt/loops/searchReader.txt",
		hash: "#!/searchR",
		elem: [],
		action: null,
		save: null
	},
};

//var config = {
//	"configloop" : {},
//	"configsection"	: {},
//	"configoutput" : {},
//};

var config = [];
/*
	TODO: сделать функции для инициализации количества разделов
	и вхождения шлейфов и сенсорных датчиков в разделы
*/

var sectionArray = [1]; /*отображает созданные разделы*/
(function (){
	for (let ix = 1; ix != Q_ALARM_LOOP + 1 + 28; ++ix) {
		sectionArray.push(0);
	}
})();


var sensorsSectionArray = []; /*вхождения шлейфов в разделы*/
(function (){
	for (let ix = 0; ix != Q_ALARM_LOOP; ++ix) {
		sensorsSectionArray.push(0);
	}
	let rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach (
			function(item) {
				var res = getSensorsParamc(item);
				if (res) {
					sensorsSectionArray[parseInt(res[1]) + Q_ALARM_LOOP] = 0;
				}
			}
		);
	}
})();

function getSensorsParamc (item) {
	var res = /^addr=([\d]+)&name=([\d]+)&mode=([\d]+)&check_param=([\d]+)&time_delay_guard=([\d]+)&delay_alarm=([\d]+)&delay_recovery_alarm=([\d]+)/.exec(item);
	return res;
}

/*
 *настройка шлейфов сигнализации
 *функции, обработчики событий
 */
var loopObj = [];
for (let ix = 0; ix != Q_ALARM_LOOP; ++ix)
	loopObj[ix] = {
		"change" : false,
		"mode": null,
		"check_param": null,
		"delay_recovery_alarm" : null,
		"delay_alarm" : null,
		"time_delay_guard": null,
		"range0": null,
		"range1": null,
		"range2": null,
		"range3": null,
		"range4": null,
		"section": 1,
	};

const AUTOAVILABLE_FROM_UNAVAILABLE = 0;
const AUTOAVILABLE_FROM_ALERT = 1;
const DONT_ACCESS_OFF_GUARD = 2;
const CONTROL_SHC_BREAK = 3;
const CONTROL_TEN_PEC = 4;
const TIME_INTEGRATION = 5;

const CHECK_MASK_MASS = [
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
];
/*******************************************************************************
 * общие функции
 ******************************************************************************/
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


function BoL() {
	cgiRequests["version"].actionFrom();
	insertShowsLoops();

	getParamCfg ("configloop");
	getParamCfg ("configsection");
	getParamCfg ("configoutput");

	getLoopValue();
	getLoopState();
	getLoopGuardState();
	getOutputState();
	getInputState();
}

function getLoopValue() {
	let rq = ajquery (cgiRequests["loopvalue"].requestFrom);
	if (rq) {
		rq = rq.split('\n');
		let flag = 1;
		rq.forEach (
			function (item) {
				if (item == null || item == undefined || item == "") return;

				let ix = cgiRequests["loopvalue"].regular.exec(item);
				let loop = document.getElementById("id_" + ix[1] + "_254_" + ix[2]);
				if (loop == null || loop == undefined) {
					flag = 0;
					return;
				}

				if (ix[3] === "off") loop.value = "отключен";
				else loop.value = ix[3];
			}
		);
		if (flag) setTimeout(function() { getLoopValue()}, 100);
	}
}

function getOutputState() {
	let rq = ajquery (cgiRequests["outputstate"].requestFrom);
	if (rq) {
		rq = rq.split('\n');
		let flag = 1;
		rq.forEach (
			function (item) {
				if (item == null || item == undefined || item == "") return;

				let ix = cgiRequests["outputstate"].regular.exec(item);
				let loop = document.getElementById("id_" + ix[1] + ix[2]);

				if (loop == null || loop == undefined) {
					flag = 0;
					return;
				}

				if (ix[3] === "1") loop.checked = true;
				else loop.checked = false;
			}
		);
		if (flag) setTimeout(function() { getOutputState()}, 233);
	}
}

function getInputState() {
	let rq = ajquery (cgiRequests["inputstate"].requestFrom);
	if (rq) {
		rq = rq.split('\n');
		let flag = 1;
		rq.forEach (
			function (item) {
				if (item == null || item == undefined || item == "") return;

				let ix = cgiRequests["inputstate"].regular.exec(item);
				let loop = document.getElementById("id_" + ix[1] + ix[2]);

				if (loop == null || loop == undefined) {
					flag = 0;
					return;
				}

				if (ix[3] === "1") loop.checked = true;
				else loop.checked = false;
			}
		);
		if (flag) setTimeout(function() { getInputState()}, 333);
	}
}

const adc_loopState = [
	"кор. зам.",
	"тревога",
	"норма",
	"тревога",
	"датчик вскрытия",
	"обрыв",
	"отключен"
];

const sens_state = [
	"норма",
	"тревога",
	"неисправность"
];
sens_state[0xfe] = "нет связи";

function getLoopState() {
	let rq = ajquery (cgiRequests["loopstate"].requestFrom);
	if (rq) {
		rq = rq.split('\n');
		let flag = 1;
		rq.forEach (
			function (item) {
				if (item == null || item == undefined || item == "") return;

				let ix = cgiRequests["loopstate"].regular.exec(item);

				if (ix) {
					let loop = document.getElementById("id_" + ix[1] + ix[2]);
					if (loop == null || loop == undefined) {
						flag = 0;
						return;
					}

					loop.value = adc_loopState[parseInt(ix[3])];
				}
				else {
					ix = /^(\w+)_(\d+)=(\d{1,6}) (\d{1,6})/.exec(item);
					if (ix) {
						let loop = document.getElementById("id_state_sens" + ix[2]);
						if (loop == null || loop == undefined) {
							flag = 0;
							return;
						}

						loop.value = sens_state[parseInt(ix[3])];

						for (let ix_stb = 0; ix_stb != 8; ++ix_stb) {
							let stb = document.getElementById("id_stb"+ix[2]+"_"+ix_stb);
							if (stb) {
								if (ix[4] & (1 << ix_stb)) stb.checked = true;
								else stb.checked = false;
							}
						}
					}
				}
			}
		);
		if (flag) setTimeout(function() { getLoopState()}, 100);
	}
}

function link_menu(param) {
	var content = ajquery(pages[param].content);
	if(content) {
		var pbdy = document.getElementById('content');
		pbdy.value = "";
		pbdy.innerHTML = content;
		document.location.hash = pages[param].hash;

		if (param == "configloop"){
			insertConfigLoops();

			configEventFunction();
		}
		else if (param == "configoutput") {
			insertConfigOutput();

			let def = document.getElementById("id_default");
			def.onclick = setDefaultOutputParam;

			let app = document.getElementById("id_apply");
			app.onclick = function () {
				ajquery ("/applyparam.cgi");
			};

			for (let ix = 0; ix != Q_OUTPUT; ++ix) {
				let br = document.getElementById("id_out"+(ix+1));
				br.onchange = function () {
					handlerOtherState("id_program"+(ix), "id_out"+(ix+1));
				}

				let br1 = document.getElementById("id_program"+(ix));
				br1.onchange = function () {
					handlerOtherState("id_program"+(ix), "id_out"+(ix+1));
				}
			}

			sectionPopUpFormed();
		}
		else if (param == "controlsensors") {
			insertTableControl();
			configEventGuard();
			getLoopGuardState();
		}
		else if (param == "search") {
			addEventSearch();
			getCurrentDevice();
		}
		else if (param == "searchR") {
			reader.getCurrentReaders();
			reader.configEventOnPage();
		}

		if (pages[param].action)
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
			if(form.name === 'form_searchR'){checkAuth();}
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

function checkAuth() {
	var textPattern=/^[a-zA-Z0-9]{4,17}$/;
	var str = "?";

	pages["login"].elem.forEach( function(item) {
		var el = document.getElementById("id_" + item);
		var tmp = el.value.match(textPattern);

		str += item+"="+tmp+"&";
	});

	var rq = ajquery (cgiRequests["checkauth"].requestFrom + str.substring(0, str.length - 1));
	rq = rq.replace(/(\r\n|\n|\r)/gm,"")
	if (rq === "OK!") {
		getParamCfg ("configloop");
		getParamCfg ("configsection");
		getParamCfg ("configoutput");

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

/*******************************************************************************
 * получает параметры и заполняет массивы на основе данных
 ******************************************************************************/
function getParamCfg(elem) {
	var rq = ajquery (cgiRequests[elem].requestFrom);
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach(
			function(item) {
				var res = cgiRequests[elem].regular.exec(item);
				if (!res) {
					res = /^(\w+)_(\d+)=(\d{1,6})/.exec(item);
				}

				if (res) {
					if (config[elem] === null || config[elem] === undefined)
						config[elem] = new Object;

					if (elem === "configsection") {
						if (res[2] >= Q_ALARM_LOOP) parseInt(res[2]) + Q_ALARM_LOOP;
						config[elem][res[1] + res[2]] = res[3];
					}
					else {
						config[elem][res[1]] = res[2];
					}
				}
			}
		);
	}
}
/*

*/
function getParam(elem) {
	var rq = ajquery (cgiRequests[elem].requestFrom);
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach(
			function(item) {
				var res = cgiRequests[elem].regular.exec(item);

				if (res) {
					if (res[1] in config[elem])
						config[elem][res[1]] = res[2];

					let el = document.getElementById("id_" + res[1]);
					el.value = res[2];
					cgiRequests[elem].callback(el);
				}
			}
		);
	}
}
/*******************************************************************************
 * получить от контроллера информацию о шлейфах и построить на её основе
 * страницу по заданному шаблону.
 ******************************************************************************/
function getParamSection(elem) {
	var rq = ajquery (cgiRequests[elem].requestFrom);
	if (rq) {
		var t = document.getElementById("id_section_table");
		while (t != null && t != undefined && t.rows.length > 1) {
	        t.deleteRow(1);
	    }

		for (let ix_t = 1; ix_t != sectionArray.length; ++ ix_t)
			sectionArray[ix_t] = 0;

		var mass = new Array();
		mass = rq.split('\n');

		mass.forEach (
			function(item) {
				var res = cgiRequests[elem].regular.exec(item);

				let loop;
				if (!res) {
					res = /^(\w+)_(\d+)=(\d{1,6})/.exec(item);
					if (res) {
						loop = parseInt(res[2]) + Q_ALARM_LOOP;
					}
					else {
						return;
					}
				}
				else {
					loop = res[2];
				}
				let sect = res[3];

				if (res[1] + res[2] in config[elem])
					config[elem][res[1] + res[2]] = res[3];

				sensorsSectionArray[loop] = sect;

				if (!sectionArray[sect]) {
					var ix  = sect;

					let be = 0;
					for (let i = ix ; i != 0; --i)
						if (sectionArray[i] == 1)
							++be;

					sectionArray[ix] = 1;

					var a = document.getElementById("id_section_table");
					var newRow = a.insertRow(be+1);

					var newCell = newRow.insertCell(0);

					var str = '\
					<table align="left" border="0" width="100%" cellpadding="1">\
						<tbody id="id_section_table'+ix+'">\
							<tr>\
								<td>\
									<input class="hide width25" id="'+ix+'" type="checkbox"/>\
									<label for="'+ix+'" class="width75">Раздел №'+ix+'</label>\
								\
									<div>\
									\
									<div>\
									<table border="0" width="100%" cellpadding="3">\
										<tbody id="id_table'+ix+'">\
										</tbody>\
									</table>\
									</div>\
												\
									<div>\
									<input type="button" id="id_add_sensors'+ix+'" class="show_popup width35"  href="#" value="Выбрать датчики" \
										onclick="showPopUpE('+(ix)+')"/>\
									</div>\
								</td>\
								<td>\
										<a class="del_x" href="#" onclick="deleteSectionRowInTbl(id_section_table'+ix+', '+ix+')">Close</a>\
								</td>\
							</tr>\
						</tbody>\
					</table>\
					';

					newCell.innerHTML = str;
				}
				redrawSensorsInSection();
			}
		);
	}

	var ht = document.getElementById("id_sensors_table");

	rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach (
			function(item) {
				var res = getSensorsParamc(item);
				if (res) {
					ht.innerHTML += '\
					<div>\
						<label for="id_ev' + (parseInt(res[1]) + Q_ALARM_LOOP)+'" class="width45">'+nameDev[parseInt(res[2])]+' № '+res[1]+':</label>\
						<label id="id_section' + (parseInt(res[1]) + Q_ALARM_LOOP)+'" for="id_ev' + (parseInt(res[1]) + Q_ALARM_LOOP)+'" class="width25"></label>\
						<input id="id_ev' + (parseInt(res[1]) + Q_ALARM_LOOP)+'" type="checkbox"/>\
					</div>\
					';
				}
			}
		);
	}
}

var sensorNumberRelay = new Array(6);

/**
	заглавная страница, на которой отображаются состояния
	шлейфов сигнализации
	интерфейсных датчиков
	выходов
	считывателя

	разметка страница формируется на стороне клиента с помощью js
*/
function insertShowsLoops () {
	var a = document.getElementById('id_loops');
	a.innerHTML = "";

	var str = "";

	for (let ix = 0; ix != Q_ALARM_LOOP; ++ix) {
		str += ' \
			<div class="loop_tbl">\
				<div class="margin_bot_2" ><label for="id_loop_254_'+(ix)+'">ШС'+(ix+1)+':</label></div>\
				<div id="id_led_stateguard_254_'+(ix)+'" class="cyrcle_guard_off"></div>\
				<div class="border1px ctrl_div_elem_height"><label id="id_stateguard_254_'+ix+'" class="width95 ctrl_word_break cntrl_align ctrl_elem">Снят с охраны</label></div>\
				<div><input id="id_state'+(ix)+'" class="width95 margin_top_2" type="text" readonly/></div>\
				<div><input id="id_loop_254_'+(ix)+'" class="width95 margin_top_2" type="text" readonly name="loop'+(ix)+'" value="" /></div>\
			</div>\
		';
	}
	a.innerHTML = str;

	var sen;
	var count_sens = 0;
	let rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach (
			function(item) {
				var res = getSensorsParamc(item);
				if (res) {
					if (count_sens % 7 == 0) {
						if (str != "" && sen) {
							sen.innerHTML = str;
						}

						sen = document.createElement('div');
						sen.id = "id_sel_" + count_sens;
						sen.className = "base"
						a.parentNode.appendChild(sen);
						sen.innerHTML = "";

						str = "";
					}
					++count_sens;
					switch (parseInt(res[2])) {
						case 1:
							str +=' \
							<div class="loop_tbl">\
								<div class="border1px ctrl_div_elem_height"><label class="width95 center-please ver_aligne"/>'+nameDev[res[2]]+' № '+res[1]+'</label></div>\
								<div id="id_led_stateguard'+(res[1])+'" class="cyrcle_guard_off"></div>\
								<div class="border1px ctrl_div_elem_height"><label id="id_stateguard'+res[1]+'" class="width95 ver_aligne ctrl_elem">Снят с охраны</label></div>\
								<div><input id="id_state_sens'+(res[1])+'" class="width95 margin_top_2" type="text" readonly/></div>\
								<div><label for="id_stb'+(res[1])+'_0">Режим работы:</label>\
									<input id="id_stb'+(res[1])+'_0" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_1">Уровень сигнала:</label>\
									<input id="id_stb'+(res[1])+'_1" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_2">Наличие связи:</label>\
									<input id="id_stb'+(res[1])+'_2" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_3">Корпус:</label>\
									<input id="id_stb'+(res[1])+'_3" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_4">Питание:</label>\
									<input id="id_stb'+(res[1])+'_4" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_5">Модуляция:</label>\
									<input id="id_stb'+(res[1])+'_5" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_6">Уровень шума:</label>\
									<input id="id_stb'+(res[1])+'_6" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_7">Уровень сигнала:</label>\
									<input id="id_stb'+(res[1])+'_7" type="checkbox"/></div>\
							</div>\
							';
							break;
						case 2:
							str +=' \
							<div class="loop_tbl">\
								<div class="border1px ctrl_div_elem_height"><label class="width95 center-please ver_aligne"/>'+nameDev[res[2]]+' № '+res[1]+'</label></div>\
								<div id="id_led_stateguard'+(res[1])+'" class="cyrcle_guard_off"></div>\
								<div class="border1px ctrl_div_elem_height"><label id="id_stateguard'+res[1]+'" class="width95 ver_aligne ctrl_elem">Снят с охраны</label></div>\
								<div><input id="id_state_sens'+(res[1])+'" class="width95 margin_top_2" type="text" readonly/></div>\
								<div><label for="id_stb0">Режим работы:</label>\
									<input id="id_stb'+(res[1])+'_0" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_1">Корпус:</label>\
									<input id="id_stb'+(res[1])+'_1" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_2">Синхронизация:</label>\
									<input id="id_stb'+(res[1])+'_2" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_3">Питание:</label>\
									<input id="id_stb'+(res[1])+'_3" type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_4" hidden>Питание:</label>\
									<input id="id_stb'+(res[1])+'_4" hidden type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_5" hidden>Модуляция:</label>\
									<input id="id_stb'+(res[1])+'_5" hidden type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_6" hidden>Уровень шума:</label>\
									<input id="id_stb'+(res[1])+'_6" hidden type="checkbox"/></div>\
								<div><label for="id_stb'+(res[1])+'_7" hidden>Уровень сигнала:</label>\
									<input id="id_stb'+(res[1])+'_7" hidden type="checkbox"/></div>\
							</div>\
							';
							break;
						default:
							break;
					}

				}
			}
		);
	}
	if (sen)
		sen.innerHTML = str;


	a = document.getElementById('id_out');
	a.innerHTML = "";
	str = "";

	for (let ix = 0; ix != Q_OUTPUT; ++ix) {
		str += '\
			<div style="block">\
				<div class="margin_bot_2" ><label for="id_out'+(ix)+'">'+(ix+1)+':</label></div>\
				<input id="id_out'+(ix)+'" disabled type="checkbox"/>\
			</div>\
		';
	}

	a.innerHTML = str;

	a = document.getElementById('id_in');
	a.innerHTML = "";
	str = "";

	str += '<div>\
			1 - BOOT<br>\
			2 - DEF_NET<br>\
			3 - DEF_PARAM<br>\
			4 - TAMPER<br>\
			</div>\
		';
	for (let ix = 0; ix != Q_INPUT; ++ix) {
		str += '\
			<div style="block">\
				<div class="margin_bot_2" ><label for="id_in'+(ix)+'">'+(ix+1)+':</label></div>\
				<input id="id_in'+(ix)+'" disabled type="checkbox"/>\
			</div>\
		';
	}


	a.innerHTML = str;
}
/*****************************************************************************************************/
var enShuffle = true;

function handlerOtherState(selId, checkId, funct, ix) {
	var selId = document.getElementById(selId);
	var checkId = document.getElementById(checkId);

	if (ix === undefined)
		ix = null;
	if (funct === undefined)
		funct = null;
	else {
		funct(selId, checkId, ix);
	}

	if (selId.selectedIndex == 0)
		checkId.checked = false;

	var a = document.getElementById("id_save");
	a.style.visibility = "visibile"
}

function callbackConfigLoops(sId, cId, ix) {
	cId.checked = true;

	var a = document.getElementById('id_delay_alarm'+(ix));
	var	b = document.getElementById('id_delay_alarm_lb'+(ix+1));
	var	c = document.getElementById('id_time_delay_guard'+(ix));
	var	d = document.getElementById('id_time_delay_guard_lb'+(ix+1));
	var e = document.getElementById('id_range3'+(ix));
	var i = document.getElementById('id_tamper_lb'+(ix+1));

	a.hidden = true;
	b.hidden = true;
	c.hidden = true;
	b.parentNode.style = "display:none"
	d.hidden = true;
	d.parentNode.style = "display:none"
	e.hidden = true;
	i.hidden = true;

	switch (sId.selectedIndex) {
		case 0:
			cId.checked = false;
			break;
		case 1:
			break;
		case 2:
			e.hidden = false;
			i.hidden = false;
			break;
		case 3:
			a.hidden = false;
			b.hidden = false;
			c.hidden = false;
			d.hidden = false;
			b.parentNode.style = ""
			d.parentNode.style = ""
			break;
		case 4:
			break;
		default:
			cId.checked = false;
			break;
	}
};

function onchangeCtrlSC_Break (ix) {
	var a = document.getElementById('id_check_param_3_'+(ix+1));
	var b = document.getElementById('id_range0'+(ix));
	var c = document.getElementById('id_range4'+(ix));
	var d = document.getElementById('id_sc_lb'+(ix+1));
	var e = document.getElementById('id_break_lb'+(ix+1));

	if (a.checked) {
		b.hidden = false;
		c.hidden = false;
		d.hidden = false;
		e.hidden = false;
	}
	else {
		b.hidden = true;
		c.hidden = true;
		d.hidden = true;
		e.hidden = true;
	}
}


var isAllRight = false;
function onblurConfigLoops (bossId, callId, ix, sc) {
	var bossId = document.getElementById(bossId);
	var callId = document.getElementById(callId);
	var sc = document.getElementById(sc);

	var a = document.getElementById('id_range0'+(ix));
	var b = document.getElementById('id_range1'+(ix));
	var c = document.getElementById('id_range2'+(ix));
	var d = document.getElementById('id_range3'+(ix));
	var e = document.getElementById('id_range4'+(ix));
	a.style.background = '#fff';
	b.style.background = '#fff';
	c.style.background = '#fff';
	d.style.background = '#fff';
	e.style.background = '#fff';

	switch (bossId.selectedIndex) {
		case 2:
			break;
		case 0:
		case 1:
		case 3:
		case 4:
		default:
			d.value = e.value;
			break;
	}

	if (!sc.checked) {
		a.value = b.value;
		if (bossId.selectedIndex != 2)
			d.value = c.value;
		e.value = c.value;
	}

	var cnt = 0;
	if (Number(a.value) > Number(b.value)) {
		if (sc.checked) {
			callId.focus();
			callId.style.background = '#FF4D4D';
			++cnt;
		}
	}
	if (Number(b.value) >= Number(c.value)) {
		callId.focus();
		callId.style.background = '#FF4D4D';
		++cnt;
	}
	if (Number(c.value) > Number(d.value)) {
			callId.focus();
			callId.style.background = '#FF4D4D';
			++cnt;
	}
	if (Number(c.value) > Number(e.value)) {
		if (bossId.selectedIndex != 2 && sc.checked) {
			callId.focus();
			callId.style.background = '#FF4D4D';
			++cnt;
		}
	}
	if (Number(d.value) > Number(e.value)) {
		if (bossId.selectedIndex == 2 && sc.checked) {
			callId.focus();
			callId.style.background = '#FF4D4D';
			++cnt;
		}
	}

	if (cnt == 0)
		isAllRight = true;
	else
		isAllRight = false;

	var f = document.getElementById("id_loop_err"+(ix));
	if (isAllRight == false) {
		f.style.display = 'table';
		f.innerHTML = "ВНИМАНИЕ! Некорректно задан диапазон";
	}
	else {
		f.style.display = 'none';
	}
}



function onClickWrite () {
	var msg = "?";

	for (let ix = 0; ix != Q_ALARM_LOOP; ++ix) {
		let str = "ix = " + ix + "\n";

		if (loopObj[ix].change) {
			for (let key in loopObj[ix]) {
				if (loopObj[ix][key] != null && key != "change") {
					str += key + " = " + loopObj[ix][key] + "\n";
					msg += key + ";" + (ix) + "="+ loopObj[ix][key] + "&";
					if ( (key + (ix+1)) in config["configloop"])
						config["configloop"][key + (ix+1)] = loopObj[ix][key];
				}
			}
//			alert(str);
		}
	}

	if (msg == "?") return;
	var rq = ajquery (cgiRequests["configloop"].requestTo + msg);

	rq = rq.replace(/(\r\n|\n|\r)/gm,"")
	if (rq == "OK!") {
		alert("param is set OK!");
	}
	else {
		alert("param is not set!");
	}
}

function setDefaultLoopParam () {
	var rq = ajquery (cgiRequests["configloop"].requestTo + "?default");

	rq = rq.replace(/(\r\n|\n|\r)/gm,"")
	if (rq == "OK!") {
		alert("param is set OK!");
	}
	else {
		alert("param is not set!");
	}
}

function setDefaultOutputParam() {
	var rq = ajquery (cgiRequests["configoutput"].requestTo + "?default");

	rq = rq.replace(/(\r\n|\n|\r)/gm,"")
	if (rq == "OK!") {
		alert("param is set OK!");
	}
	else {
		alert("param is not set!");
	}
}

function onchangeTest (id, ix_loop, ix_param) {
	if (ix_param === undefined)
		ix_param = null;

	if (id.id != "id_loop"+(ix_loop))
		loopObj[ix_loop].change = true;

	if (ix_param != null) {
		switch (ix_param) {
			case 0: case 1: case 2: case 3: case 4:
				if (id.checked)
					loopObj[ix_loop].check_param = loopObj[ix_loop].check_param | CHECK_MASK_MASS[ix_param];
				else
					loopObj[ix_loop].check_param = loopObj[ix_loop].check_param & ~CHECK_MASK_MASS[ix_param];
				break;
			case 5:
				if (id.selectedIndex == 1)
					loopObj[ix_loop].check_param = loopObj[ix_loop].check_param | CHECK_MASK_MASS[ix_param];
				else
					loopObj[ix_loop].check_param = loopObj[ix_loop].check_param & ~CHECK_MASK_MASS[ix_param];
				break;
		}
	} else {
		if (id.id == "id_time_delay_guard"+(ix_loop))
			loopObj[ix_loop].time_delay_guard = id.value;
		if (id.id == "id_delay_alarm"+(ix_loop))
			loopObj[ix_loop].delay_alarm = id.value;
		if (id.id == "id_delay_recovery_alarm"+(ix_loop))
			loopObj[ix_loop].delay_recovery_alarm = id.value;
		if (id.id == "id_mode"+(ix_loop))
			loopObj[ix_loop].mode = id.value;
		if (id.id == "id_range0"+(ix_loop))
			loopObj[ix_loop].range0 = id.value;
		if (id.id == "id_range1"+(ix_loop))
			loopObj[ix_loop].range1 = id.value;
		if (id.id == "id_range2"+(ix_loop))
			loopObj[ix_loop].range2 = id.value;
		if (id.id == "id_range4"+(ix_loop))
			loopObj[ix_loop].range4 = id.value;
		if (id.id == "id_range3"+(ix_loop))
			loopObj[ix_loop].range3 = id.value;
	}
}

function configEventFunction () {
	var wr = document.getElementById("id_save");
	wr.onclick = onClickWrite;

	let app = document.getElementById("id_apply");
	app.onclick = function () {
		ajquery ("/applyparam.cgi");
	};

	let def = document.getElementById("id_default");
	def.onclick = setDefaultLoopParam;

	for (let ix = 0; ix != Q_ALARM_LOOP; ++ix) {
		let tmp = document.getElementById("id_loop"+(ix));
		tmp.onchange = function() {
			handlerOtherState("id_mode"+(ix), "id_loop"+(ix));
			onchangeTest(tmp, ix);
		}

		let tmp2 = document.getElementById("id_mode"+(ix));

		tmp2.addEventListener("DOMSubtreeModified", function() {
			handlerOtherState("id_mode"+(ix), "id_loop"+(ix), callbackConfigLoops, (ix));
			onchangeTest(tmp2, ix);
		});

		tmp2.onchange = function() {
			handlerOtherState("id_mode"+(ix), "id_loop"+(ix), callbackConfigLoops, (ix));
			onchangeTest(tmp2, ix);
		}

		pages["configloop"].elem.forEach(
				function (item) {
					if (item == "check_param") {
						/*пройтись по всем битам параметра check_param*/
						for (let iy = 0; iy != 6; ++iy) {
							let tst = document.getElementById("id_"+item+"_"+(iy)+"_"+(ix+1));
							tst.onchange = function() {
								onchangeTest(tst, ix, iy);
								if (iy == 3)
									onchangeCtrlSC_Break((ix));
							}
						}
					}
					else {
						let sc = document.getElementById("id_"+item+""+(ix));
						sc.onchange = function() {
							onchangeTest(sc, ix);
						}
						sc.onblur = function () {
							onblurConfigLoops("id_mode"+(ix), "id_"+item+""+(ix), (ix), "id_check_param_3_"+(ix+1));
						}
					}
				}
		);
	}
}

function insertConfigLoops () {
	var a = document.getElementById('id_loops');
	a.innerHTML = "";

	var str = "";

	for (let ix = 0; ix != Q_ALARM_LOOP; ++ix) {
		str += '\
<div>\
	<input class="hide width25" id="id_loop'+(ix)+'" type="checkbox" />\
	<label for="id_loop'+(ix)+'" class="width25">ШС №'+(ix+1)+'</label>\
	<select class="width50" name="mode" id="id_mode'+(ix)+'" size="1">\
		<option selected value="0">Отключен</option>\
		<option value="1">Охранный</option>\
		<option value="2">Охранный с датчиком вскрытия</option>\
		<option value="3">Охранный входной</option>\
		<option value="4">Тревожный</option>\
	</select>\
	<div>\
		<input id="id_check_param'+(ix)+'" hidden>\
		<div class="fieldset_div">\
			<label for="id_check_param_0_'+(ix+1)+'" class="width75">Автоперевзятие из невзятия:</label>\
			<input class="width25" id="id_check_param_0_'+(ix+1)+'" type="checkbox"/>\
		</div>\
		<div class="fieldset_div">\
			<label for="id_check_param_1_'+(ix+1)+'" class="width75">Автоперевзятие из тревоги:</label>\
			<input class="width25" id="id_check_param_1_'+(ix+1)+'" type="checkbox"/>\
		</div>\
		<div class="fieldset_div">\
			<label for="id_check_param_2_'+(ix+1)+'" class="width75">Без права снятия с охраны:</label>\
			<input class="width25" id="id_check_param_2_'+(ix+1)+'" type="checkbox"/>\
		</div>\
		<div class="fieldset_div">\
			<label for="id_check_param_3_'+(ix+1)+'" class="width75">Контроль обрыва и КЗ:</label>\
			<input class="width25" id="id_check_param_3_'+(ix+1)+'" type="checkbox" checked />\
		</div>\
		<div class="fieldset_div">\
			<label for="id_check_param_4_'+(ix+1)+'" class="width75">Контроль 10% отклонений:</label>\
			<input class="width25" id="id_check_param_4_'+(ix+1)+'" type="checkbox"/>\
		</div>\
		<div class="fieldset_div">\
			<label for="id_check_param_5_'+(ix+1)+'" class="width75">Время интегрирования:</label>\
			<select class="width25" name="mode" id="id_check_param_5_'+(ix+1)+'" size="1">\
				<option value="s1">70 мс</option>\
				<option selected value="s2">300 мс</option>\
			</select>\
		</div>\
		<div class="fieldset_div">\
			<label for="id_time_delay_guard'+(ix)+'" id="id_time_delay_guard_lb'+(ix+1)+'" class="width75">Задержка перехода под охрану, сек:</label>\
			<input class="width25" id="id_time_delay_guard'+(ix)+'" type="number"  name="minAmpl_2" autocomplete="off" value="" />\
		</div>\
		<div class="fieldset_div">\
			<label for="id_delay_alarm'+(ix)+'" id="id_delay_alarm_lb'+(ix+1)+'" class="width75">Задержка перехода в тревогу, сек:</label>\
			<input class="width25" id="id_delay_alarm'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
		</div>\
		<div class="fieldset_div">\
			<label for="id_delay_recovery_alarm'+(ix)+'" id="id_delay_recovery_alarm_lb'+(ix+1)+'" class="width75">Задержка восстановления из тревоги, сек:</label>\
			<input class="width25" id="id_delay_recovery_alarm'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
		</div>\
		<div class="fieldset_div">\
			<div class="loop_tbl">\
				<label class="width75">Границы диапазонов, Ом:</label>\
				<div class="error" id="id_loop_err'+(ix)+'"></div>\
				<div class="fieldset_div">\
					<label id="id_sc_lb'+(ix+1)+'" for="id_range0'+(ix)+'" class="width_rangen">КЗ</label>\
<input class="width_rangen" id="id_range0'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="200" />\
					<label id="id_low_lb'+(ix+1)+'" for="id_range1'+(ix)+'" class="width_rangen">Тревога</label>\
<input class="width_rangen" id="id_range1'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="2300"/>\
					<label id="id_hight_lb'+(ix+1)+'" for="id_range2'+(ix)+'" class="width_rangen">Норма</label>\
<input class="width_rangen" id="id_range2'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="9400"/>\
					<label id="id_hight_lb'+(ix+1)+'" for="id_range2'+(ix)+'" class="width_rangen">Тревога</label>\
<input class="width_rangen" id="id_range3'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="14100"/>\
					<label id="id_tamper_lb'+(ix+1)+'" for="id_range3'+(ix)+'" class="width_rangen">Вскрытие</label>\
<input class="width_rangen" id="id_range4'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="51000"/>\
					<label id="id_break_lb'+(ix+1)+'" for="id_range4'+(ix)+'" class="width_rangen">Обрыв</label>\
				</div>\
			</div>\
		</div>\
	</div>\
</div>\
		';
	}

	a.innerHTML = str;
}


/*******************************************************************************
 * Конфигурация выходов
 ******************************************************************************/
function insertConfigOutput () {
	var a = document.getElementById('id_out');
	a.innerHTML = "";

	var str = "";

	var rq = ajquery (cgiRequests["configoutput"].requestFrom);
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach(
			function(item) {
				var res = /^program([\d]+)=(\d{1,10})/.exec(item);
				if (res) {
					let ix = parseInt(res[1]);
					let id_prog = parseInt(res[2]);

					if (id_prog == 3) {
						str +=' \
							<input class="hide width25" id="id_out'+(ix+1)+'" type="checkbox" />\
							<label for="id_out'+(ix+1)+'" class="width25">Выход №'+(ix+1)+'</label>\
							<select class="width50" name="mode" id="id_program'+(ix)+'" size="1" >\
								<option selected value="0">Не управлять</option>\
								<option value="1">Включить</option>\
								<option value="2">Отключить</option>\
								<option value="3">Охранная лампа</option>\
								<option value="4">Сирена</option>\
								<option value="5">Дистанционный контроль</option>\
							</select>\
							<div>\
								<div hidden>\
									<label id="id_out_t_ctrl_lb'+(ix+1)+'" for="id_time_ctrl'+(ix)+'" class="width65">Время управления, сек:</label>\
									<input class="width35" id="id_time_ctrl'+(ix)+'" type="number"  name="minAmpl_2" autocomplete="off" value="" />\
								</div>\
								<div hidden>\
									<label id="id_out_t_period_lb'+(ix+1)+'" for="id_period_ctrl'+(ix)+'" class="width65">Периодичность управления, сек:</label>\
									<input class="width35" id="id_period_ctrl'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
								</div>\
								<table border=\'1\' width="100%" cellpadding="5">\
									<tr>\
										<th>Источники</th><th>Состояния</th><th></th>\
									</tr>\
									<tbody id="id_table'+(ix+1)+'"></tbody>\
								</table>\
								<div hidden>\
									<input class="width35" id="id_source_sensors'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
									<input class="width35" id="id_source_sensors_h'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
								</div>\
								<div hidden>\
									<input class="width35" id="id_source_states'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
								</div>\
							</div>\
						';
					}
					else {
						str +=' \
							<input class="hide width25" id="id_out'+(ix+1)+'" type="checkbox" />\
							<label for="id_out'+(ix+1)+'" class="width25">Выход №'+(ix+1)+'</label>\
							<select class="width50" name="mode" id="id_program'+(ix)+'" size="1" >\
								<option selected value="0">Не управлять</option>\
								<option value="1">Включить</option>\
								<option value="2">Отключить</option>\
								<option value="3">Охранная лампа</option>\
								<option value="4">Сирена</option>\
								<option value="5">Дистанционный контроль</option>\
							</select>\
							<div>\
								<div class="fieldset_div">\
									<label id="id_out_t_ctrl_lb'+(ix+1)+'" for="id_time_ctrl'+(ix)+'" class="width65">Время управления, сек:</label>\
									<input class="width35" id="id_time_ctrl'+(ix)+'" type="number"  name="minAmpl_2" autocomplete="off" value="" />\
								</div>\
								<div class="fieldset_div">\
									<label id="id_out_t_period_lb'+(ix+1)+'" for="id_period_ctrl'+(ix)+'" class="width65">Периодичность управления, сек:</label>\
									<input class="width35" id="id_period_ctrl'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
								</div>\
								<table border=\'1\' width="100%" cellpadding="5">\
									<tr>\
										<th>Источники</th><th>Состояния</th><th></th>\
									</tr>\
									<tbody id="id_table'+(ix+1)+'"></tbody>\
								</table>\
								<div hidden>\
									<input class="width35" id="id_source_sensors'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
									<input class="width35" id="id_source_sensors_h'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
								</div>\
								<div hidden>\
									<input class="width35" id="id_source_states'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off" value="" />\
								</div>\
							</div>\
						';
					}
				}
			}
		);
	}

	a.innerHTML = str;

	for (let ix = 0; ix != Q_OUTPUT; ++ix) {
		addRow('id_table'+(ix+1), ix);
	}

	a = document.getElementById('id_loops_out');

	var rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach (
			function(item) {
				var res = getSensorsParamc(item);
				if (res) {
					a.innerHTML += '\
					<div>\
						<label for="id_loop' + (parseInt(res[1]) + Q_ALARM_LOOP)+'">'+nameDev[parseInt(res[2])]+' № '+res[1]+':</label>\
						<input id="id_loop' + (parseInt(res[1]) + Q_ALARM_LOOP)+'" type="checkbox"/>\
					</div>\
					';
				}
			}
		);
	}

	for (let ix_t = 1; ix_t != sectionArray.length; ++ ix_t)
			sectionArray[ix_t] = 0;

	var rq = ajquery (cgiRequests["configsection"].requestFrom);
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach(
			function(item) {
				var res = cgiRequests["configsection"].regular.exec(item);
				if (!res) {
					res = /^(\w+)_(\d+)=(\d{1,6})/.exec(item);
					if (res) {
						res[2] = parseInt(res[2]) + Q_ALARM_LOOP;
					}
				}
				if (res) {
					let sect = res[3];
					let loop = res[2];

					if (res[1] + res[2] in config["configsection"])
						config["configsection"][res[1] + res[2]] = res[3];

					sensorsSectionArray[loop] = sect;

					if (!sectionArray[sect]) {
						var ix  = sect;

						let be = 0;
						for (let i = ix ; i != 0; --i)
							if (sectionArray[i] == 1)
								++be;

						sectionArray[ix] = 1;
					}
				}
			}
		);
	}
}

var ix_row = 0;
function addRow(id_tbl, ix) {
	var id_tabel = document.getElementById(id_tbl);
	var ix_row = id_tabel.rows.length;
	var newRow=id_tabel.insertRow(ix_row);

	var newCell = newRow.insertCell(0);

	var str = '\
			<div id="id_tbl_celS'+(ix+1)+'"></div>\
			<input type="button" id="id_change_source'+(ix+1)+'" class="show_popup width75" rel="reg_form" href="#" value="Добавить источники" onclick="showPopUpS(\''+String(ix+1).toString()+'\')"/>\
	';

	newCell.innerHTML = str;

	newCell = newRow.insertCell(1);

	str = '\
		<div id="id_tbl_cel'+(ix+1)+'"></div>\
		<input type="button" id="id_change_event'+(ix+1)+'" class="show_popup width75" rel="reg_form" href="#" value="Добавить состояния" onclick="showPopUpE(\''+(String(ix+1)).toString()+'\')"/>\
	';
	newCell.innerHTML = str;

	newCell = newRow.insertCell(2);

	str = '<a class="del_x" href="#" onclick="deleteRow(id_tbl_celS'+(ix+1)+', id_tbl_cel'+(ix+1)+')">Close</a>';
	newCell.innerHTML = str;
}

function deleteRow(id_tbl, id_el) {
	id_el.innerHTML="";
	id_tbl.innerHTML="";
}

function showPopUpE(t) {
	var a = document.getElementById("popup_a");
	a.style.display = "block";
	var b = document.getElementById("hiden_in");
	b.value = t;
	/*TODO: сделать для адресных датчиков -  адрес датчика = индекс в массиве*/
	for (let ix = 0; ix != sensorsSectionArray.length; ++ix) {
		if (ix in sensorsSectionArray == false) continue;

		let c = document.getElementById("id_section"+(ix));
		if (c) {
			if (sensorsSectionArray[ix] == 0)
				c.innerHTML = "Без раздела";
			else
				c.innerHTML = "Раздел №" + sensorsSectionArray[ix];
		}
	}
}

function showPopUpS(t) {
	var a = document.getElementById("popup_b");
	a.style.display = "block";
	var b = document.getElementById("hiden_inS");
	b.value = t;
}

function hidePopUpS() {
	var a = document.getElementById("popup_b");
	a.style.display = "none";
}

function insertSensor() {
	var b = document.getElementById("hiden_inS");

	var t = document.getElementById('id_tbl_celS'+b.value) ;

	t.value = "";
	var b1 = "";

	let rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
	}

	let sens_hide = document.getElementById ('id_source_sensors' + (b.value-1));
	let sens_hide_h = document.getElementById ('id_source_sensors_h' + (b.value-1));
	sens_hide.value = 0;
	sens_hide_h.value = 0;

	for (let ix = 0; ix != sensorsSectionArray.length; ++ix) {
		if (ix in sensorsSectionArray == false) continue;

		let lb = document.getElementById("id_loop"+(ix));

		if (lb.checked) {
			if (ix >= Q_ALARM_LOOP) {
				if (ix > 31) {
					sens_hide_h.value |= 1 << ix - 32 - Q_ALARM_LOOP;
				}
				else {
					sens_hide.value |= 1 << ix;
				}
			}
			else
			{
				sens_hide.value |= 1 << ix;
			}

			b1 += '<div id="id_div';
			b1 += ix;
			if (ix >= Q_ALARM_LOOP) {

				mass.forEach (
					function(item) {
						var res = getSensorsParamc(item);
						if (res) {
							let iy = parseInt(res[1]) + Q_ALARM_LOOP;
							if (iy == ix) {
								b1 += '">	<label><b>'+nameDev[res[2]]+' № '+res[1];
							}
						}
					}
				);
			}
			else {
				b1 += '">	<label><b>Шлейф № ';
				b1 += ix+1;
			}
			b1 += '</b></label>';
		}
	}

	t.innerHTML = b1;

	for (let ix = 0; ix != sensorsSectionArray.length; ++ix) {
		if (ix in sensorsSectionArray == false) continue;

		let lb = document.getElementById("id_loop"+(ix));
		lb.checked = false;
		lb = document.getElementById("id_section"+(ix));
		if (lb)
			lb.checked = false;
	}

	t = document.getElementById("id_change_source"+b.value);
	t.value = "Изменить события";
	hidePopUpS();
}

var stateSensorsList = [
	"Отключен",
	"Снят с охраны",
	"Взят на охрану",
	"Тревога",
	"Невзятие",
	"Неисправность",
	"Задержка перехода\
	под охрану",
	"Задержка перехода \
	в тревогу",
	"Задержка восстановления\
	из тревоги",
	"Потеря связи"
	];

function addEvent() {
	var b = document.getElementById("hiden_in");

	var t = document.getElementById('id_tbl_cel'+b.value) ;

	t.value = "";
	var b1 = "";
	let ev_hide = document.getElementById ('id_source_states' + (b.value-1))
	ev_hide.value = 0;
	for (let ix = 0; ix != stateSensorsList.length; ++ix) {

		let lb = document.getElementById("id_ev"+(ix));

		if (lb && lb.checked) {
			ev_hide.value |= 1 << ix;
			b1 += '<div id="id_div';
			b1 += ix;
			b1 += '">	<label><b>'
			b1 += stateSensorsList[ix];
			b1 += '</b></label>';
		}
	}

	t.innerHTML = b1;

	for (let ix = 0; ix != stateSensorsList.length; ++ix) {
		let lb = document.getElementById("id_ev"+(ix));
		if (lb)
			lb.checked = false;
	}

	t = document.getElementById("id_change_event"+b.value);
	t.value = "Изменить события";
	hidePopUp();
}

function closePopUp () {
	for (let ix = 0; ix != stateSensorsList.length; ++ix) {
		let lb = document.getElementById("id_ev"+(ix));
		if (lb)	lb.checked = false;
	}
	let btn = document.getElementById("id_default");
	if (btn) btn.disabled = false;

	btn = document.getElementById("id_save");
	if (btn) btn.disabled = false;

	hidePopUp();
}

function showPopUp() {
	var a = document.getElementById("popup_a");
	a.style.display = "block";
}

function setOutputParam () {
	var str = "?";
	var flags = 0;


	for (let ix = 0; ix != Q_OUTPUT; ++ix) {
		let val = document.getElementById("id_program" + ix);
		str += "program;" + ix + "=" + val.value + "&";
		val = document.getElementById("id_period_ctrl" + ix);
		str += "period_ctrl;" + ix + "=" + val.value + "&";

		val = document.getElementById("id_time_ctrl" + ix);
		str += "time_ctrl;" + ix + "=" + val.value + "&";

		val = document.getElementById("id_source_sensors" + ix);
		str += "source_sensors;" + ix + "=" + val.value + "&";

		val = document.getElementById("id_source_sensors_h" + ix);
		str += "source_sensors_h;" + ix + "=" + val.value + "&";

		val = document.getElementById("id_source_states" + ix);
		str += "source_states;" + ix + "=" + val.value + "&";
	}

	var rq = ajquery (cgiRequests["configoutput"].requestTo + str);

	rq = rq.replace(/(\r\n|\n|\r)/gm,"")
	if (rq == "OK!") {
		alert("param is set OK!");
	}
	else {
		alert("param is not set!");
	}
}
/******************************************************************************/

/*******************************************************************************
 * Конфигурация разделов
 ******************************************************************************/
function showPopUpSS() {
	let btn = document.getElementById("id_default");
	btn.disabled = true;

	btn = document.getElementById("id_save");
	if (btn) btn.disabled = true;

	var a = document.getElementById("popup_s");
	a.style.display = "block";

	var b = document.getElementById("id_sel_sec");

	var bb = document.getElementById("id_sel");

	if (bb != null && b != undefined) {
		b.removeChild(bb)
	}

	var sel = document.createElement('select');
	sel.id = "id_sel";
	let ix_sel = 0;
	for (let ix = 1; ix != sectionArray.length + 1; ++ix)
		if (sectionArray[ix] == 0)
			sel.options[ix_sel++] = new Option(ix, ix);

	var button = document.getElementById("id_save_section")
	b.insertBefore(sel, button); //appendChild(sel)
}

function addSectionSensors() {
	var b = document.getElementById("hiden_in");

	let tmp = new Array(sensorsSectionArray.length);

	for (let ix = 0; ix != sensorsSectionArray.length; ++ix) {
		tmp[ix] = 0;
	}

	for (let ix = 0; ix != sensorsSectionArray.length; ++ix) {
		if (ix in sensorsSectionArray == false) continue;

		let lb = document.getElementById("id_ev"+(ix));

		if (lb.checked) {
			sensorsSectionArray[ix] = b.value;
			tmp[ix] = b.value;
		}
	}

	for (let ix = 0; ix != sensorsSectionArray.length; ++ix) {
		if (ix in sensorsSectionArray == false) continue;

		if (sensorsSectionArray[ix] == b.value)
			if (tmp[ix] != b.value)
				sensorsSectionArray[ix] = 0;
	}

	redrawSensorsInSection();

	for (let ix = 0; ix != sensorsSectionArray.length; ++ix) {
		if (ix in sensorsSectionArray == false) continue;

		let lb = document.getElementById("id_ev"+(ix));
		lb.checked = false;
	}

	let btn = document.getElementById("id_default");
	if (btn) btn.disabled = false;

	btn = document.getElementById("id_save");
	if (btn) btn.disabled = false;
	hidePopUp();
}

function insertSectionRaw () {
	var a = document.getElementById("id_section_table");

	sectionArray[0] = 1;

	var ix_row = a.rows.length;

	if (ix_row > sectionArray.length - 1) {
		return;
	}
	var nsection = document.getElementById('id_sel');
	var ix  = nsection.value;

	let be = 0;
	for (let i = ix ; i != 0; --i)
		if (sectionArray[i] == 1)
			++be;

	sectionArray[ix] = 1;

	var newRow = a.insertRow(be+1);

	var newCell = newRow.insertCell(0);

	var str = '\
	<table align="left" border="0" width="100%" cellpadding="1">\
		<tbody id="id_section_table'+ix+'">\
			<tr>\
				<td>\
					<input class="hide width25" id="'+ix+'" type="checkbox"/>\
					<label for="'+ix+'" class="width75">Раздел №'+ix+'</label>\
				\
					<div>\
					\
					<div>\
					<table border="0" width="100%" cellpadding="3">\
						<tbody id="id_table'+ix+'">\
						</tbody>\
					</table>\
					</div>\
								\
					<div>\
					<input type="button" id="id_add_sensors'+ix+'" class="show_popup width35"  href="#" value="Выбрать датчики" \
						onclick="showPopUpE('+(ix)+')"/>\
					</div>\
				</td>\
				<td>\
						<a class="del_x" href="#" onclick="deleteSectionRowInTbl(id_section_table'+ix+', '+ix+')">Close</a>\
				</td>\
			</tr>\
		</tbody>\
	</table>\
	';

	newCell.innerHTML = str;

	closePopUpSS();

	let btn = document.getElementById("id_default");
	btn.disabled = true;

	btn = document.getElementById("id_save");
	btn.disabled = true;

	showPopUpE(ix);
}

function redrawSensorsInSection () {

	for (let ix = 0; ix != sectionArray.length; ++ix) {
		let t = document.getElementById('id_table'+ix);
		while (t != null && t != undefined && t.rows.length > 0) {
	        t.deleteRow(0);
	    }
	}

	let rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
	}

	for (let ix = 0; ix != sectionArray.length; ++ix) {
		let t = document.getElementById('id_table'+ix);

		if (t == null || t == undefined) continue;

		for (let iy = 0; iy != Q_ALARM_LOOP; ++iy) {
			if (iy in sensorsSectionArray == false) continue;

			if (sensorsSectionArray[iy] == ix) {
				var newRow = t.insertRow();

				var newCell = newRow.insertCell(0);

				let b1 = '<div id="id_div';
				b1 += ix;
				b1 += iy;
				b1 += '">	<label><b>Шлейф сигнализации № ';
				b1 += iy+1;
				b1 += '</b></label>';

				newCell.innerHTML = b1;

				newCell = newRow.insertCell(1);

				if (ix > 0) {
					var ix_rows = t.rows.length;
					b1 = '\
		<a class="del_x" href="#" onclick="deleteSectionRaw(id_table'+ix+', id_div'+ix+iy+', '+iy+')">Close</a>\
					';
					newCell.innerHTML = b1;
				}
			}
		}

		mass.forEach (
			function(item) {
				var res = getSensorsParamc(item);
				if (res) {
					let iy = parseInt(res[1]) + Q_ALARM_LOOP;
					if (sensorsSectionArray[iy] == ix) {
						var newRow = t.insertRow();
						var newCell = newRow.insertCell(0);

						let b1 = '<div id="id_div';
						b1 += ix;
						b1 += iy;
						b1 += '">	<label><b>'+nameDev[res[2]]+' № '+res[1];
						b1 += '</b></label>';

						newCell.innerHTML = b1;

						newCell = newRow.insertCell(1);

						if (ix > 0) {
							var ix_rows = t.rows.length;
							b1 = '\
				<a class="del_x" href="#" onclick="deleteSectionRaw(id_table'+ix+', id_div'+ix+iy+', '+iy+')">Close</a>\
							';
							newCell.innerHTML = b1;
						}
					}
				}
			}
		);
	}
}

function deleteSectionRaw(id_tbl, ix_row, ix) {
	id_tbl.deleteRow(ix_row.parentNode.parentNode.rowIndex);
	sensorsSectionArray[ix] = 0;

	redrawSensorsInSection();
}

function deleteSectionRowInTbl (id_tbl, ix) {
	id_tbl.parentNode.parentNode.parentNode.parentNode.deleteRow ( id_tbl.parentElement.parentNode.parentNode.sectionRowIndex);

	for (let i = 0; i != sensorsSectionArray.length; ++i) {
		if (i in sensorsSectionArray == false) continue;

		if (sensorsSectionArray[i] == ix)
			sensorsSectionArray[i] = 0;
	}

	sectionArray[ix] = 0;

	redrawSensorsInSection();
}

function closePopUpSS () {
	let btn = document.getElementById("id_default");
	btn.disabled = false;

	btn = document.getElementById("id_save");
	btn.disabled = false;

	hidePopUpSS();
}

function hidePopUpSS() {
	var a = document.getElementById("popup_s");
	a.style.display = "none";
}

function setSectionParam () {
	var str = "?";
	var flags = 0;

	for (let ix = 0; ix != Q_ALARM_LOOP; ++ix)
		str += "loop_254;" + ix + "=" + sensorsSectionArray[ix] + "&";

	for (let ix = Q_ALARM_LOOP; ix != sensorsSectionArray.length; ++ix) {
		if (ix in sensorsSectionArray == false) continue;

		str += "loop_" + (ix - Q_ALARM_LOOP) + "=" + sensorsSectionArray[ix] + "&";
	}

	var rq = ajquery (cgiRequests["configsection"].requestTo + str);

	rq = rq.replace(/(\r\n|\n|\r)/gm,"")
	if (rq == "OK!") {
		alert("param is set OK!");
	}
	else {
		alert("param is not set!");
	}
}
/******************************************************************************/


function hidePopUp() {
	var a = document.getElementById("popup_a");
  a.style.display = "none";
}







function hidePopUpSS() {
	var a = document.getElementById("popup_s");
  a.style.display = "none";
}



function closePopUpS () {
	for (let ix = 0; ix != sensorsSectionArray.length; ++ix) {
		if (ix in sensorsSectionArray == false) continue;

		let lb = document.getElementById("id_loop"+(ix));
		if (lb) lb.checked = false;

		lb = document.getElementById("id_section"+(ix));
		if (lb)
			lb.checked = false;
	}
	hidePopUpS();
}



/*******************************************************************************
 * постановка на охрану / снятие с охраны
 ******************************************************************************/

 const nameDev = [  "Неизвестное устройство",
						"STS-107",
						"STS-123",
						"STS-103",
						"STS-125",
						"STS-119",
						"STS-111"
					];

const btnNameList = ["Поставить на охрану", "Снять с охраны"];

function insertTableControl () {
	var a = document.getElementById('id_ctrl_guard');
	if (a == null || a == undefined)
		return;

	for (let ix_t = 1; ix_t != sectionArray.length; ++ ix_t)
			sectionArray[ix_t] = 0;

	var rq = ajquery (cgiRequests["configsection"].requestFrom);
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach(
			function(item) {
				var res = cgiRequests["configsection"].regular.exec(item);
				if (!res) {
					res = /^(\w+)_(\d+)=(\d{1,6})/.exec(item);
					if (res) {
						res[2] = parseInt(res[2]) + Q_ALARM_LOOP;
					}
				}
				if (res) {
					let sect = res[3];
					let loop = res[2];

					if (res[1] + res[2] in config["configsection"])
						config["configsection"][res[1] + res[2]] = res[3];

					sensorsSectionArray[loop] = sect;

					if (!sectionArray[sect]) {
						var ix  = sect;

						let be = 0;
						for (let i = ix ; i != 0; --i)
							if (sectionArray[i] == 1)
								++be;

						sectionArray[ix] = 1;
					}
				}
			}
		);
	}

	a.innerHTML = "";

	var str = "<div>";

	for (let ix_s = 0; ix_s != sectionArray.length; ++ix_s) {
		if (sectionArray[ix_s] == 0) continue;

		if (ix_s == 0) {
			str += '\
			<div>\
			<button id="id_getg'+ix_s+'" class="width45 ctrl_elem"> Взять на охрану&#13;&#10;нераспределенные датчики</button>\
			<input type="button" class="width8 display_none"/>\
			<button id="id_getug'+ix_s+'" class="width45 ctrl_elem">Снять с охраны&#13;&#10;нераспределенные датчики</button>\
			<\div>\
			'
		}
		else {
			str += '\
			<div>\
			<button id="id_getg'+ix_s+'" class="width45">Раздел'+(ix_s)+' взять на охрану</button>\
			<input type="button" class="width8 display_none"/>\
			<button id="id_getug'+ix_s+'" class="width45">Раздел'+(ix_s)+' снять с охраны</button>\
			<\div>\
			'
		}
	}

	str += "</div>";

	for (let ix = 0; ix != Q_ALARM_LOOP; ++ix) {
		str +=' \
			<div class="ctrl_tbl_string cntrl_tbl_brd_bot">\
			<div class="width35 border1px ctrl_div_elem_height"><label class="width95 center-please ver_aligne"/>Шлейф сигнализации №'+(ix + 1)+'</label></div>\
			<div id="id_led_stateguard_254_'+(ix)+'" class="cyrcle_guard_off"></div>\
			<div class="width25 border1px ctrl_div_elem_height"><label id="id_stateguard_254_'+ix+'" class="width95 ver_aligne ctrl_elem">Снят с охраны</label></div>\
			<div class="width25"><button id="id_setonguard_254_'+ix+'" class="width95 cntrl_align ctrl_elem"/>Поставить на охрану</div>\
		</div>\
		';
	}

	rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach (
			function(item) {
				var res = getSensorsParamc(item);
				if (res) {
					str +=' \
						<div class="ctrl_tbl_string cntrl_tbl_brd_bot">\
						<div class="width35 border1px ctrl_div_elem_height"><label class="width95 center-please ver_aligne"/>'+nameDev[res[2]]+' № '+res[1]+'</label></div>\
						<div id="id_led_stateguard'+(res[1])+'" class="cyrcle_guard_off"></div>\
						<div class="width25 border1px ctrl_div_elem_height"><label id="id_stateguard'+res[1]+'" class="width95 ver_aligne ctrl_elem">Снят с охраны</label></div>\
						<div class="width25"><button id="id_setonguard'+res[1]+'" class="width95 cntrl_align ctrl_elem"/>Поставить на охрану</div>\
					</div>\
					';
				}
			}
		);
	}

	a.innerHTML = str;

	for (let ix_s = 0; ix_s != sectionArray.length; ++ix_s) {
		if (sectionArray[ix_s] == 0) continue;

		let msg_on = "?";
		let msg_off = "?";

		for (let ix_l = 0; ix_l != sensorsSectionArray.length; ++ix_l) {
			if (ix_l in sensorsSectionArray == false) continue;


			if (sensorsSectionArray[ix_l] == ix_s) {
				if (ix_l >= Q_ALARM_LOOP) {
					msg_on += "onguard_" + (ix_l - Q_ALARM_LOOP) + "&";
					msg_off += "offguard_" + (ix_l - Q_ALARM_LOOP) + "&";
				}
				else {
					msg_on += "onguard_254_" + ix_l + "&";
					msg_off += "offguard_254_" + ix_l + "&";
				}
			}
		}

		let btn_on = document.getElementById("id_getg"+ix_s);
		btn_on.onclick = function() {
			let r = ajquery (cgiRequests["ctrlsensor"].requestTo + msg_on);
			toggleName(1);
		}

		let btn_off = document.getElementById("id_getug"+ix_s);
		btn_off.onclick = function() {
			let r = ajquery (cgiRequests["ctrlsensor"].requestTo + msg_off);
			toggleName(1);
		}
	}

	toggleName(1);
}

function configEventGuard () {

	for (let ix = 0; ix != Q_ALARM_LOOP; ++ix) {
		let btn = document.getElementById("id_setonguard_254_"+ix);

		if (btn == null || btn == undefined)
			break;
		//----------------------------------------------------------------------
		btn.onclick = function () {

			if (btn.textContent == btnNameList[0]) {
				let r = ajquery (cgiRequests["ctrlsensor"].requestTo + "?onguard_254_"+ix);
				toggleName(1);//btn.textContent = btnNameList[1];
			}
			else {
				let r = ajquery (cgiRequests["ctrlsensor"].requestTo + "?offguard_254_"+ix);
				toggleName(1);//btn.textContent = btnNameList[0];
			}
		}
	};

	var rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach (
			function(item) {
				var res = getSensorsParamc(item);
				if (res) {
					let btn = document.getElementById("id_setonguard"+res[1]);

					if (btn == null || btn == undefined)
						return;
					//----------------------------------------------------------------------
					btn.onclick = function () {

						if (btn.textContent == btnNameList[0]) {
							let r = ajquery (cgiRequests["ctrlsensor"].requestTo + "?onguard_"+res[1]);
							toggleName(1);//btn.textContent = btnNameList[1];
						}
						else {
							let r = ajquery (cgiRequests["ctrlsensor"].requestTo + "?offguard_"+res[1]);
							toggleName(1);//btn.textContent = btnNameList[0];
						}
					}
				}
			}
		);
	}
		//----------------------------------------------------------------------
	let aon = document.getElementById("id_all_on");
	aon.onclick = function () {
		let rq = ajquery (cgiRequests["controlsensors"].requestTo + "?all_onguard");

		rq = rq.replace(/(\r\n|\n|\r)/gm,"")
		if (rq == "OK!") {
			toggleName(1);
			//alert("param is set OK!");
		}
		else {
			alert("param is not set!");
		}
	};

	let aoff = document.getElementById("id_all_off");
	aoff.onclick = function () {
		let rq = ajquery (cgiRequests["controlsensors"].requestTo + "?all_offguard");

		rq = rq.replace(/(\r\n|\n|\r)/gm,"")
		if (rq == "OK!") {
			toggleName(0);
		//	alert("param is set OK!");
		}
		else {
			alert("param is not set!");
		}
	};
}

function toggleName(dir) {
	var r = ajquery (cgiRequests["ctrlsensor"].requestFrom);

	if (r) {
		r = r.split('\n');
		r.forEach(
			function(item) {
				if (item == "")
					return;
				let reg = /^([\w]+)_([\d]+)_([\d]+)/.exec(item)
				let btn;
				if (reg) {
					btn = document.getElementById("id_setonguard_254_"+reg[3]);
				}
				else {
					reg = /^([\w]+)_([\d]+)/.exec(item)
					if (reg) btn = document.getElementById("id_setonguard"+reg[2]);
				}

				if (btn) {
					if (reg[1] == "onguard") {
					btn.textContent = btnNameList[1];
					}
					else if (reg[1] == "offguard") {
						btn.textContent = btnNameList[0];
					}
				}
			}
		);
	}
}
/******************************************************************************/

function applyParam() {
	let rq = ajquery (cgiRequests["apply"].requestFrom);

	rq = rq.replace(/(\r\n|\n|\r)/gm,"")
	if (rq == "OK!") {
		toggleName(1);
		alert("param is set OK!");
	}
	else {
		alert("param is not set!");
	}
}

/******************************************************************************/
var timerID_getLoopGuardState = 0;
function getLoopGuardState () {
	let rq = ajquery (cgiRequests["loopguardstate"].requestFrom);

	if (rq) {
		rq = rq.split('\n');
		let flag = 1;

		rq.forEach (
			function (item) {
				if (item == null || item == undefined || item == "") return;

				let ix = cgiRequests["loopguardstate"].regular.exec(item);
				let loop_lbl;
				let loop
				if (ix) {
					loop_lbl = document.getElementById("id_" + ix[1] + "_254_" + ix[3]);
					loop = document.getElementById("id_led_" + ix[1] + "_254_" + ix[3]);
				}
				else {
					ix = /^(\w+)_(\d+)=(\d{1,6})/.exec(item);
					loop_lbl = document.getElementById("id_" + ix[1] + ix[2]);
					loop = document.getElementById("id_led_" + ix[1] + ix[2]);
					ix[4] = ix[3];
				}

				if (loop == null || loop == undefined) {
					flag = 0;
					return;
				}

				loop_lbl.innerText = stateSensorsList[parseInt(ix[4])];

				switch (parseInt(ix[4])) {
					case 0:
						loop.className = "cyrcle_off";
						break;
					case 1:
						loop.className = "cyrcle_guard_off"
						break;
					case 2:
						loop.className = "cyrcle_guard_on"
						break;
					case 3:
						loop.className = "cyrcle_alarm"
						break;
					case 4:
						loop.className = "cyrcle_alarm red_animation_unavailable"
						break;
					case 5:
						loop.className = "cyrcle_alarm red_animation_fault"
						break;
					case 6:
						loop.className = "cyrcle_guard_on green_animation_guard_on"
						break;
					case 7:
						loop.className = "cyrcle_alarm green_animation_alarm_on"
						break;
					case 8:
						loop.className = "cyrcle_guard_on green_animation_recovery"
						break;
					case 9:
						loop.className = "cyrcle_guard_off green_animation_rec_tmp"
						break;
					default:
						loop.className = "cyrcle_off"
						break;
				}
			}
		);

		toggleName(1);
		if (flag) {
			if (timerID_getLoopGuardState == 1) {
				clearTimeout(timerID_getLoopGuardState);
				timerID_getLoopGuardState = 0;
			}
			timerID_getLoopGuardState = setTimeout(function() { getLoopGuardState()}, 100);
		}
	}
}

function sectionPopUpFormed () {
	var rq = ajquery (cgiRequests["configsection"].requestFrom);
	if (rq) {
		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach(
			function(item) {
				var res = cgiRequests["configsection"].regular.exec(item);
				if (res) {
					let sect = res[3];
					let loop = res[2];

					if (res[1] + res[2] in config["configsection"])
						config["configsection"][res[1] + res[2]] = res[3];

					sensorsSectionArray[loop] = sect;
				}
			}
		);
	}

	let str = "";

	for (let ix_s = 0; ix_s != sectionArray.length; ++ix_s) {
		if (sectionArray[ix_s] == 0) continue;

		if (ix_s == 0) {
			str += '\
				<div>\
					<label for="id_section'+ix_s+'">Нераспределенные:</label>\
					<input id="id_section'+ix_s+'" type="checkbox" onchange="sectionClickCallback('+ix_s+', id_section'+(ix_s)+', '+ix_s+');"/>\
				</div>\
			'
		}
		else {
			str += '\
				<div>\
					<label for="id_section'+ix_s+'">Раздел № '+ix_s+':</label>\
					<input id="id_section'+ix_s+'" type="checkbox" onchange="sectionClickCallback('+ix_s+', id_section'+(ix_s)+', '+ix_s+');"/>\
				</div>\
			'
		}
	}

	let so = document.getElementById("id_section_out");
	so.innerHTML = "";
	so.innerHTML = str;
}

function sectionClickCallback(ix_s, id_m, sensSection) {
	if (id_m.checked) {
		for (let ix_l = 0; ix_l != sensorsSectionArray.length; ++ix_l) {
			if (ix_l in sensorsSectionArray == false) continue;

			if (sensorsSectionArray[ix_l] == ix_s) {
				let a = document.getElementById("id_loop"+(ix_l));
				a.checked = true;
			}
		}
	}
	else {
		for (let ix_l = 0; ix_l != sensorsSectionArray.length; ++ix_l) {
			if (ix_l in sensorsSectionArray == false) continue;

			if (sensorsSectionArray[ix_l] == ix_s) {
				let a = document.getElementById("id_loop"+(ix_l));
				a.checked = false;
			}
		}
	}
}
/*****************************************************************/
var ix_sensors = 0;

function srchd(ix, stop_ix) {
	let rq = ajquery ("/searchdev.cgi?addr="+ix);
	if (rq) {
		if (rq == "NOPSUP") {

		}
		else if (rq != "FAIL") {
			var mass = new Array();
			mass = rq.split('\n');
			mass.forEach (
				function(item) {
					var res = /^addr=([\d]+)&type=([\d]+)/.exec(item);
					if (res) {
						if (res[1] && res[2]) {
							for (let ix = 1; ix <= ix_sensors; ++ix) {
								let addr = document.getElementById("id_addr" + ix);

								if (addr && addr.value == res[1]) {
									addr.classList.remove("back_red");
									addr.classList.add("back_green");
									return;
								}
							}

							addNewSensorsOnPage();

							let elem = document.getElementById("id_addr" + ix_sensors);
							if (elem) {
								elem.value = res[1];
								elem.classList.add("back_yellow");
							}
							elem = document.getElementById("id_name" + ix_sensors);
							if (elem) elem.value = res[2];
						}
					}
				}
			);
		}
	}

	let ix_s = document.getElementById("id_search_txt");
	if (ix != stop_ix) {
		ix_s.innerHTML = ix+1;
		setTimeout(function() { srchd(++ix, stop_ix)}, 20);
		return;
	}
	ix_s.innerHTML = "";
	let btn = document.getElementById('id_auto_search');
	btn.disabled = false;
}

function clearSearch () {
	let inhtml = document.getElementById('id_sens_mass');
	inhtml.innerHTML = "";
	ix_sensors = 0;
}

function searchDevice() {
	let btn = document.getElementById('id_auto_search');
	btn.disabled = true;

	let l = document.getElementById("id_start_addr");
	let h = document.getElementById("id_end_addr");

	getCurrentDevice();
	for (let ix = 1; ix <= ix_sensors; ++ix) {
		let addr = document.getElementById("id_addr" + ix);

		addr.classList.add("back_red");
	}

	let ix_s = document.getElementById("id_search_txt");
	ix_s.innerHTML = parseInt(l.value);
	srchd(parseInt(l.value), parseInt(h.value));
}

function saveDevice () {
	let elem = document.getElementById("id_addr");
	let str = "/savesearchsens.cgi?addr=" + elem.value;
	elem = document.getElementById("id_name");
	str += "&name=" + elem.value;

	elem = document.getElementById("id_mode");
	str += "&mode=" + elem.value;
	elem = document.getElementById("id_check_param");
	str += "&check_param=" + elem.value;
	elem = document.getElementById("id_time_delay_guard");
	str += "&time_delay_guard=" + elem.value;
	elem = document.getElementById("id_delay_alarm");
	str += "&delay_alarm=" + elem.value;
	elem = document.getElementById("id_delay_recovery_alarm");
	str += "&delay_recovery_alarm=" + elem.value;

	let rq = ajquery (str);
	if (rq == "BE") {
		alert("Такое устройство уже добавлено");
	}
	else if (rq == "FULL") {
		alert("Добавлено максимальное количество устройств");
	}
	else if (rq != "OK") {
		alert("Ошибка");
	}
	link_menu('search');
}

function saveDevices () {
	let str = "/savesearchsensors.cgi?";//"/savesearchsensors.cgi?";

	for (let ix = 1; ix <= ix_sensors; ++ix) {
		let del = document.getElementById("id_del"+ix);
		if (del.value === "На удаление") continue;

		let elem = document.getElementById("id_addr"+ix);
		if (str === "") {
			str += "addr=" + elem.value;
		}
		else {
			str += "&addr=" + elem.value;
		}
		elem = document.getElementById("id_name"+ix);
		str += "&name=" + elem.value + "&";

		elem = document.getElementById("id_mode"+ix);
		str += "&mode=" + elem.value;
		elem = document.getElementById("id_check_param"+ix);
		str += "&check_param=" + elem.value;
		elem = document.getElementById("id_time_delay_guard"+ix);
		str += "&time_delay_guard=" + elem.value;
		elem = document.getElementById("id_delay_alarm"+ix);
		str += "&delay_alarm=" + elem.value;
		elem = document.getElementById("id_delay_recovery_alarm"+ix);
		str += "&delay_recovery_alarm=" + elem.value;
	}

	let rq = ajquery (str);
	if (rq == "OK") {
		;//alert("Устройство добавлено успешно");
	}
	else {
		alert("Ошибка");
	}
	link_menu('search');
}

function addEventSearch() {
	let l = document.getElementById("id_start_addr");
	let h = document.getElementById("id_end_addr");

	l.addEventListener("blur", function() {minMaxCheck(l);pairDiffCheck(l, h);});

	h.addEventListener("blur", function() {minMaxCheck(h);pairDiffCheck(l, h);});

	let ad = document.getElementById("id_auto_search");
	ad.addEventListener("click", function() {searchDevice();})
}

function minMaxCheck (id) {
	if (parseInt(id.min) > parseInt(id.value)) id.value = id.min;
	else if (parseInt(id.max) < parseInt(id.value)) id.value = id.max;
}

function pairDiffCheck (id_l, id_h) {
	if (parseInt(id_l.value) > parseInt(id_h.value)) id_l.value = id_h.value;
}


function addNewSensorsOnPage () {
	++ix_sensors;

	let ediv = document.createElement('div');
	ediv.id = "id_div"+ix_sensors;
	ediv.innerHTML = '\
		<div class="fieldset_div">\
			<label class="width70">Адрес</label>\
			<input type="text" id="id_addr'+ix_sensors+'" class="width30" disabled="true">\
		</div>\
		<div class="fieldset_div">\
			<label class="width70">Тип устройства</label>\
			<select class="width30" id="id_name'+ix_sensors+'" size="1">\
				<option selected value="0">Неизвестность</option>\
				<option value="1">STS-107</option>\
				<option value="2">STS-123</option>\
			</select>\
		</div>\
		<div>\
			<input class="hide width25" id="id_loop'+ix_sensors+'" type="checkbox" />\
			<label for="id_loop'+(ix_sensors)+'" class="width25">Параметры</label>\
			<select class="width50" name="mode" id="id_mode'+ix_sensors+'" size="1">\
				<option value="0">Отключен</option>\
				<option selected value="1">Охранный</option>\
				<option value="3">Охранный входной</option>\
				<option value="4">Тревожный</option>\
			</select>\
			<div>\
				<input id="id_check_param'+(ix_sensors)+'" hidden>\
				<input id="id_section'+(ix_sensors)+'" hidden value="0">\
				<div class="fieldset_div">\
					<label for="id_check_param_0_'+(ix_sensors)+'" class="width75">Автоперевзятие из невзятия:</label>\
					<input class="width25" id="id_check_param_0_'+(ix_sensors)+'" type="checkbox" onchange="setDeviceParam(this, 0, \'id_check_param'+(ix_sensors)+'\');"/>\
				</div>\
				<div class="fieldset_div">\
					<label for="id_check_param_1_'+(ix_sensors)+'" class="width75">Автоперевзятие из тревоги:</label>\
					<input class="width25" id="id_check_param_1_'+(ix_sensors)+'" type="checkbox" onchange="setDeviceParam(this, 1, \'id_check_param'+(ix_sensors)+'\');"/>\
				</div>\
				<div class="fieldset_div">\
					<label for="id_check_param_2_'+(ix_sensors)+'" class="width75">Без права снятия с охраны:</label>\
					<input class="width25" id="id_check_param_2_'+(ix_sensors)+'" type="checkbox" onchange="setDeviceParam(this, 2, \'id_check_param'+(ix_sensors)+'\');"/>\
				</div>\
				<div class="fieldset_div">\
					<label for="id_time_delay_guard'+(ix_sensors)+'" class="width75">Задержка перехода под охрану, сек:</label>\
					<input class="width25" id="id_time_delay_guard'+(ix_sensors)+'" type="number"  name="minAmpl_2" autocomplete="off" value="0" />\
				</div>\
				<div class="fieldset_div">\
					<label for="id_delay_alarm'+(ix_sensors)+'" class="width75">Задержка перехода в тревогу, сек:</label>\
					<input class="width25" id="id_delay_alarm'+(ix_sensors)+'" type="number" name="minAmpl_2" autocomplete="off" value="0" />\
				</div>\
				<div class="fieldset_div">\
					<label for="id_delay_recovery_alarm'+(ix_sensors)+'" class="width75">Задержка восстановления из тревоги, сек:</label>\
					<input class="width25" id="id_delay_recovery_alarm'+(ix_sensors)+'" type="number" name="minAmpl_2" autocomplete="off" value="0" />\
				</div>\
			</div>\
		</div>\
		<div>\
			<input type="button" id="id_del'+ix_sensors+'" class="width30" value="На запись" onclick="delDevice(this);">\
		</div>\
		<hr>\
	';

	var b = document.getElementById("id_sens_mass")
	b.appendChild(ediv);
}

function delDevice (id) {
	if (id.value === "На запись")
		id.value = "На удаление";
	else
		id.value = "На запись";
}

function getCurrentDevice () {
	var rq = ajquery ("/getcurdev.cgi");
	if (rq) {
		let inhtml = document.getElementById('id_sens_mass');
		inhtml.innerHTML = "";
		ix_sensors = 0;

		var mass = new Array();
		mass = rq.split('\n');
		mass.forEach (
			function(item) {
				var res = getSensorsParamc(item);
				if (res) {
					addNewSensorsOnPage();

					let elem = document.getElementById("id_addr" + ix_sensors);
					if (elem) elem.value = res[1];
					elem = document.getElementById("id_name" + ix_sensors);
					if (elem) elem.value = res[2];

					elem = document.getElementById("id_mode" + ix_sensors);
					if (elem) elem.value = res[3];
					elem = document.getElementById("id_check_param" + ix_sensors);
					if (elem) elem.value = res[4];
					elem = document.getElementById("id_check_param_0_" + ix_sensors);
					if (elem) {
						if (res[4] & CHECK_MASK_MASS[0]) elem.checked = true;
						else elem.checked = false;
					}
					elem = document.getElementById("id_check_param_1_" + ix_sensors);
					if (elem) {
						if (res[4] & CHECK_MASK_MASS[1]) elem.checked = true;
						else elem.checked = false;
					}
					elem = document.getElementById("id_check_param_2_" + ix_sensors);
					if (elem) {
						if (res[4] & CHECK_MASK_MASS[2]) elem.checked = true;
						else elem.checked = false;
					}

					elem = document.getElementById("id_time_delay_guard" + ix_sensors);
					if (elem) elem.value = res[5];
					elem = document.getElementById("id_delay_alarm" + ix_sensors);
					if (elem) elem.value = res[6];
					elem = document.getElementById("id_delay_recovery_alarm" + ix_sensors);
					if (elem) elem.value = res[7];
				}
			}
		);
	}
}

function setDeviceParam (id, ix_param, id_sect) {
	let dev = document.getElementById(id_sect);

	switch(ix_param) {
		case 0: case 1: case 2:
			if (id.checked)
				dev.value = dev.value | CHECK_MASK_MASS[ix_param];
			else
				dev.value = dev.value & ~CHECK_MASK_MASS[ix_param];
			break;
	}
}

function rebootDev() {
	var rebootPage = ajquery("/txt/reboot.txt");
	if (rebootPage) {
		var pbdy = document.getElementById('content');

		pbdy.innerHTML = '';
		pbdy.innerHTML = rebootPage;

		var pnet = ajquery('/rstip.cgi');
		if (pnet) {
			var mass = new Array();
			mass = pnet.split('\n');
			var reg = /^([\w^\d]+)=(\d{1,3}.\d{1,3}.\d{1,3}.\d{1,3})/

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
				rebootCmd = rebootCmd.replace(/(\r\n|\n|\r)/gm,"")
				if (rebootCmd === "OK!") {
					var tmp = mass[0].match(reg);
					var cmd = 'location.href="http://'+tmp[2]+'/index.html"';
					setTimeout(cmd, 5000);
				}
			}
		}
	}
}

/****************************************************************************************/
function Reader_t () {
	let addr = 0;
	let bytes = 5; /*количество значимых байт карты*/
	let speed = 2;
	let time_storage = 0;
	let cnt_readers = 0;
	let search_addr = [];

	let self = this;

	function hidePopUp () {
		let a = document.getElementById("popup_a");
		a.style.display = "none";
	}

	function redrawreader (addr) {
		let field = document.getElementById("id_sens_mass");
		field.innerHTML = "";

		let str = "";
		for (let ix = 0; ix != addr.length; ++ix) {
			str += '\
					<div>\
						<input class="hide width25" id="id_addr'+ix+'" type="checkbox" />\
						<label for="id_addr'+ix+'" class="width70">Адрес'+addr[ix]+'</label>\
						<div>\
							<div>\
								<input type="button" id="id_reboot_r'+ix+'" class="width50" value="Перезагрузить">\
							</div>\
							<div>\
								<input type="button" id="id_reset_addr'+ix+'" class="width50" value="Сбросить адрес устройства">\
							</div>\
							<div class="fieldset_div">\
								<label for="id_cnt_bytes'+(ix)+'" class="width75">Количество байт карты:</label>\
								<input class="width25" id="id_cnt_bytes'+(ix)+'" type="number"  name="minAmpl_2" autocomplete="off"/>\
							</div>\
							<input class="hide width25" id="id_additional'+ix+'" type="checkbox" />\
							<label for="id_additional'+ix+'" class="width70">Дополнительно</label>\
							<div>\
								<div class="fieldset_div">\
									<label for="id_new_addr'+(ix)+'" class="width75">Новый адрес устройства:</label>\
									<input hidden id="id_old_addr'+(ix)+'" value="'+addr[ix]+'" type="number"  name="minAmpl_2" autocomplete="off"/>\
									<input class="width25" id="id_new_addr'+(ix)+'" type="number"  name="minAmpl_2" autocomplete="off"/>\
								</div>\
								<div class="fieldset_div">\
									<label class="width75">Скорость считывателя</label>\
									<select class="width25" id="id_speed'+ix+'" size="1">\
										<option value="0">2400</option>\
										<option value="1">4800</option>\
										<option value="2">9600</option>\
										<option value="3">115200</option>\
										<option selected value="4"></option>\
									</select>\
								</div>\
								<div class="fieldset_div">\
									<label for="id_time_storage'+(ix)+'" class="width75">Время хранения кода карты, x100 мсек:</label>\
									<input class="width25" id="id_time_storage'+(ix)+'" type="number" name="minAmpl_2" autocomplete="off"/>\
								</div>\
								<div>\
									<input id="id_get_param'+(ix)+'" type="button" class="width30" value="Прочитать">\
									<input id="id_set_param'+(ix)+'" type="button" class="width30" value="Записать">\
								</div>\
								<hr>\
								<div class="fieldset_div">\
									<label for="id_code_card'+(ix)+'" class="width65">Код карты:</label>\
									<input class="width35" id="id_code_card'+(ix)+'" type="text" autocomplete="off"/>\
								</div>\
								<div>\
									<input id="id_get_cc'+(ix)+'" type="button" class="width30" value="Прочитать">\
								</div>\
							</div>\
						</div>\
					</div>\
					<div>\
						<input type="button" id="id_del'+ix+'" class="width30" value="На запись" onclick="delDevice(this);">\
					</div>\
					'
		}
		field.innerHTML = str;

		for (let ix = 0; ix != addr.length; ++ix) {
			let el = document.getElementById("id_get_cc"+(ix));
			el.addEventListener("click", function() {self.getLastCodeCard(addr[ix], ix);});

			el = document.getElementById("id_get_param"+(ix));
			el.addEventListener("click", function() {self.getParam(addr[ix], ix);});

			el = document.getElementById("id_set_param"+(ix));
			el.addEventListener("click", function() {self.setParam(addr[ix], ix);});

			el = document.getElementById("id_reset_addr"+(ix));
			el.addEventListener("click", function() {self.setParam(addr[ix], ix, 1);});

			el = document.getElementById("id_additional"+(ix));
			el.addEventListener("click", function() {self.getParam(addr[ix], ix);});

			el = document.getElementById("id_reboot_r"+ix);
			el.addEventListener("click", function() {self.reset(addr[ix]);});
		}
	}

	function deleteReader () {
		self.clearSearch();
		self.saveReaderAll();
	}

	this.configEventOnPage = function () {
		let l = document.getElementById("id_add_man");
		l.addEventListener("click", function() {self.showPopUp();});

		l = document.getElementById("id_close_popupr");
		l.addEventListener("click", function() {self.closePopUp();});

		l = document.getElementById("id_auto_search");
		l.addEventListener("click", function() {self.search();});

		l = document.getElementById("id_clear_search");
		l.addEventListener("click", function() {self.clearSearch();});

		l = document.getElementById("id_save");
		l.addEventListener("click", function() {self.saveReaderAll();});

		l = document.getElementById("id_apply");
		l.addEventListener("click", function() {applyParam();});

		l = document.getElementById("id_save_reader");
		l.addEventListener("click", function() {self.saveReader();});

		l = document.getElementById("id_delete");
		l.addEventListener("click", function() {deleteReader();});
	}

	this.showPopUp = () => {
		let a = document.getElementById("popup_a");
		a.style.display = "block";
	}

	this.closePopUp = () => {
		let a = document.getElementById("id_addr");
		a.value="";

		a = document.getElementById("id_bytes");
		a.value=5;

		hidePopUp();
	}

	this.clearSearch = () => {
		let inhtml = document.getElementById('id_sens_mass');
		inhtml.innerHTML = "";
	}

	this.reset = (addr) => {
		let str = "/resetreader.cgi?addr="+addr;
		ajquery (str);
	}

	this.updateReaderParam = () => {
		let str = "/updatereadparam.cgi?";
		let rq = ajquery (str);
	}

	this.getLastCodeCard = function (addr, ix) {
		let str = "/getlastcodecard.cgi?addr="+addr;
		let rq = ajquery (str);
		if (rq == "FAIL") {
			alert(rq);
		}
		else {
			let el = document.getElementById("id_code_card"+(ix));
			let cnt_b = document.getElementById("id_cnt_bytes"+ix);
			el.value = rq.substr(0, cnt_b.value * 2);
		}
	}

	this.getParam = function (addr, ix) {
		let str = "/getcurreader.cgi?addr="+addr;
		let rq = ajquery (str);
		if (rq == "FAIL") {
			alert(rq);
			return;
		}

		rq = rq.split('\n');
		for (let i =0; i < rq.length - 1; ++i) {
			let r = /^addr=(\d{1,3})&cnt_bytes=(\d)&speed=(\d)&time_storage=(\d)/.exec(rq[i]);
			if (r) {
				if (r[1] != addr) continue;

				let el = document.getElementById("id_speed"+ix);
				el.value = r[3];
				el = document.getElementById("id_time_storage"+ix);
				el.value = r[4];
			}
		}
	}

	this.setParam = (addr, ix, rst_addr = null) => {
		let str = "/setparamreaders.cgi?addr="+addr;

		if (rst_addr) {
			str += "&def_addr";
		}
		else {
			let el = document.getElementById("id_new_addr"+ix);
			str += "&new_addr=" + el.value;
			el = document.getElementById("id_speed"+ix);
			str += "&speed=" + el.value;
			el = document.getElementById("id_time_storage"+ix);
			str += "&time_storage=" + el.value;
		}

		let rq = ajquery (str);
		if (rq == "FAIL") {
			alert(rq);
			return;
		}
		else if (rq == "DEF_OK") {
			self.reset(addr);
		}
		self.getCurrentReaders();
	}

	this.saveReader = () => {
		let str = "/savereaders.cgi?";

		let a = document.getElementById("id_addr");
		str += "addr=" + a.value;

		a = document.getElementById("id_bytes");
		str += "&cnt_bytes=" + a.value;
		str += "&end";

		let rq = ajquery (str);
		if (rq == "OK") {
			alert("Устройство добавлено успешно");
		}
		else if (rq == "FULL") {
			alert("Записано максимальное количество считывателей");
		}
		else {
			alert("Ошибка");
		}
		self.closePopUp();

		self.getCurrentReaders();
	}

	this.saveReaderAll = () => {
		let str = "/savereaders.cgi?clear";

		for (let ix = 0; ix != cnt_readers; ++ix) {
			let elem = document.getElementById("id_del"+ix);
			if (elem.value === "На удаление") continue;

			elem = document.getElementById("id_old_addr"+ix);
			if (elem && elem.value) str += "&addr=" + elem.value;

			elem = document.getElementById("id_cnt_bytes"+ix);
			if (elem && elem.value) str += "&cnt_bytes=" + elem.value;

			str += "&end";
		}

		let rq = ajquery (str);
		if (rq == "OK") {
			alert("saveReaderAll");
		}
		else {
			alert("Ошибка");
		}

		self.getCurrentReaders();
	}

	this.getCurrentReaders = () => {
		let str = "/getcurreader.cgi?";
		let rq = ajquery (str);
		if (rq == "EMPTY") {
			alert("Считыватели отсутствуют в системе");
			return;
		}

		rq = rq.split('\n');
		let addr = [];
		let cnt_bytes = [];
		for (let i =0; i < rq.length - 1; ++i) {
			let r = /^addr=(\d{1,3})&cnt_bytes=(\d)/.exec(rq[i]);
			if (r) {
				addr.push(r[1]);
				cnt_bytes.push(r[2]);
			}
		}

		cnt_readers = addr.length;
		redrawreader(addr);

		for (let ix = 0; ix != cnt_readers; ++ix) {
			let elem = document.getElementById("id_new_addr"+ix);
			if (elem) elem.value = addr[ix];

			elem = document.getElementById("id_cnt_bytes"+ix);
			if (elem) elem.value = cnt_bytes[ix];
		}
	}

	this.search = function (addr, end_a) {
		let args_str = "";
		let start, end;
		if (arguments.length) {
			if (end_a < addr) {
				redrawreader(search_addr);

				for (let ix = 0; ix != search_addr.length; ++ix) {
					let elem = document.getElementById("id_new_addr"+ix);
					if (elem) elem.value = search_addr[ix];

					elem = document.getElementById("id_cnt_bytes"+ix);
					if (elem) elem.value = 5;
				}
				return;
			}
			args_str = "addr="+addr;
		}
		else {
			start = document.getElementById("id_start_addr");
			addr = start.value;
			end = document.getElementById("id_end_addr");
			end_a = end.value;
			search_addr = [];
			self.search(addr, end_a);
			return;
		}

		let q = ajquery ("/searchreader.cgi?"+args_str);
		if (q == "FAIL") {
			console.log("Ошибка search addr =" + addr);
		}
		else {
			let r = /addr=(\d{1,3})/.exec(q);
			search_addr.push(r[1]);
			console.log("search DONE " + q);
		}
		self.search(++addr, end_a);
	}
}

var reader = new Reader_t();
