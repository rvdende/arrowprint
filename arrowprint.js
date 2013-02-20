// Arrow command line relay
// this program scans for connected devices and relays communication to a socket server
// v 2.0
//ARROW PRINTER
//READS IN GCODE AND RELAYS TO DUE FOR PRINTING.
// * notes
// * F 		FEED RATE 		mm/minute


var verbose = false;
var verboseraw = true;
var lineconfirmed = 0;
var linenum = 0;
//used for checking echo'd correct data
var sendingline = "";
var echoline = "";

var datasamples = []; //we'll save our data in here
var config = {}
var fs 			= require('fs')
fs.readFile(__dirname + '/config.json', function(err,data) {
	console.log('loading')
	if (err) console.log("Error loading config json" + err)
  	if (data) {
		config = JSON.parse(data)
		startarduino();		
	}
})	

//READ IN GCODE
var gcodefile = __dirname + '/wade-big.gcode'; //filename of gcode text file with path
var gdata = [];
var gline = 0;
var gready = false;
var printerready = false;

fs.readFile(gcodefile, function(err,data) {
	var gcoderaw = data.toString();
	gdata = gcoderaw.split('\r\n');
	gready = true;
})

//
////////// ARDUINO ////////////////////////////////////////////////////////////////
//

function startarduino() {
	var io = require('socket.io-client');
	var socket = io.connect('http://'+config.domain+':'+config.httpport);
	var SerialPort = require("serialport"); //so we can access the serial port

	var arduino = 'unset';
	SerialPort.list( function (err, ports) {
		ports.forEach( function(port) {
			console.log('PORT:'+port.comName);
			console.log('ID:'+port.pnpId);		
			console.log(port.manufacturer);
			if (port.pnpId.indexOf('duino') > 0) {
				console.log('FOUND ARDUINO')

				arduino = new SerialPort.SerialPort('/dev/serial/by-id/'+port.pnpId, {
				          baudrate: config.baudrate //Make sure your arduino is set to this rate
				        });			
				setup();
			} else {
				//console.log('Unknown device! PORT:'+port.comName + 'ID:'+port.pnpId);					
			}		
		});
	});


	//CONNECTED ARDUINO STREAM
	var setup = function() {
		var createScraper = require('json-scrape');
		var scraper = createScraper();
		var counter = 0;
		var timer = Date.now();
		//Handle data from arduino serial connection
		arduino.on("data", function (data) { 
			if (verboseraw) {console.log('RAW:'+data.toString());  }
			//console.log(data)
			//console.log(data.toString().length)
			//console.log(data.toString().replace('\n','').replace('\r',''));
			scraper.write(data); 
			});

		console.log('Connected to Arduino!')

		scraper.on('data', function (obj) {

			if (obj.log) {
	    		console.log(obj.log)
	    		//arduino.write(" "); //to confirm
	    	}

	    	if (obj.version) {
	    		console.log("printer firmware version: "+ obj.version)
	    		//arduino.write(" "); 
	    	}

			if (obj.type == "pars") {
				echoline = JSON.stringify(obj);	
				console.log("pars:"+echoline);
	    	}

	    	if (obj.status) {
	    		console.log(obj.status);
	    		
	    		if ((obj.status == "waiting")&&(gready == true)) {
	    			
	    			
	    			var lineprocess = gdata[lineconfirmed];
	    			
	    			if (lineprocess[0] == ";") {
	    				lineprocess = "";
	    			}

					if (lineprocess.indexOf(";")>1) {
	    				lineprocess = lineprocess.slice(0,lineprocess.indexOf(";"));
	    			}

	    			//lineprocess.replace(" ", "");
	    			if (lineprocess[lineprocess.length-1] == " ") {
	    				lineprocess = lineprocess.slice(0,lineprocess.length-1);
	    			}

	    			if (lineprocess.length > 0) {
	    				//MAIN PREPPED
	    				//var gcodeobj = {type : "gcode"};
	    				var gcodeobj = {}
	    				//gcodeobj.line = lineprocess;
	    				console.log('process:'+lineprocess);
	    				if (lineprocess != lineprocess.split(" ")) {
	    					var linetemp = lineprocess.split(" ");
	    					for (var z in linetemp) {
	    						var tempnum = linetemp[z].slice(1,linetemp[z].length);
	    						var commannnd = linetemp[z][0];
								if ((commannnd == "X")||(commannnd == "Y")||(commannnd == "F")||(commannnd == "E")||(commannnd == "Z")) {
		    						tempnum = tempnum * 100;	
		    					}
	    						gcodeobj[linetemp[z][0]] = Math.round(tempnum);
	    					}
	    				} else {
	    					var tempnum = lineprocess.slice(1,lineprocess.length);
	    					var commannnd = lineprocess[0];
	    					if ((commannnd == "X")||(commannnd == "Y")||(commannnd == "F")||(commannnd == "E")||(commannnd == "Z")) {
	    						tempnum = tempnum * 100;	
	    					}
	    					
	    					gcodeobj[lineprocess[0]] = Math.round(tempnum);
	    				}

	    				sendingline = JSON.stringify(gcodeobj);
	    				arduino.write(sendingline);	  
						console.log("SEND:"+sendingline)	    					
						lineconfirmed++;
	    				/////////////////// 				
	    			} else { lineconfirmed++; }
	    			
	    			

	    			
	    		}

	    	}//waiting
			

		});
	}
}
