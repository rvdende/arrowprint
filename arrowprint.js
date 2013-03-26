// Arrow command line relay
// this program scans for connected devices and relays communication to a socket server
// v 5.0
//ARROW PRINTER
//READS IN GCODE AND RELAYS TO DUE FOR PRINTING.

var NEWGODE = '{"cmd":"M107"}';
var TEMP = 0;
var SerialPort = require("serialport"); //so we can access the serial port
var os = require("os");
var TIMElast = os.uptime();
var STARTTIME = os.uptime();

SerialPort.list( function (err, ports) {
	ports.forEach( function(port) {
		console.log('PORT:'+port.comName);
		console.log('ID:'+port.pnpId);		
		console.log(port.manufacturer);

		if (port.pnpId.indexOf('duino') > 0) {
			console.log('FOUND ARDUINO')
			var arduino = new SerialPort.SerialPort('/dev/serial/by-id/'+port.pnpId, {baudrate: 115200});			
			gcodeprint(arduino);
		} else {
			//console.log('scanning...');					
		}		
	});
});



var gcodeprint = function(arduino) {
	console.log('Connected to Arduino!')
	var scraper = require('json-scrape')();
	
	arduino.on("data", function (data) { 
		//RAW SERIAL IN
		//console.log(data.toString());
		scraper.write(data); 
	});
	scraper.on('data', function (obj) {
		//ARDUINO DATA JSON CONVERTER
		if (obj.status == "waiting") {
			//console.log(NEWGODE);
			arduino.write(NEWGODE); //writes to arduino
			gcodereader.resume();	//reads another line from file
		}

		if (obj.thermistor) {
			var temperature = (obj.thermistor/Math.pow(2,16))*1024;
			temperature = thermistorlookupcelsius(temperature);
			console.log("Thermistor " + temperature);
		}

	});
}


var reader = require ("buffered-reader");
var BinaryReader = reader.BinaryReader;
var DataReader = reader.DataReader;
var file = "printer_bracket.gcode";
var offset;
var linecounter = 0;

var gcodereader;

var readerstream = new DataReader (file, { encoding: "utf8" })
	    /* READS GCODE AND SENDS EACH LINE TO GET PARSED */
        .on ("error", function (error){
            console.log (error);
        })

        .on ("line", function (line){
        	linecounter++;
			var text = line.replace(/;.*$/, '').trim(); // Remove comments
			if (text) {
				//console.log(text); //RAW GCODE LINE
				this.pause();		//pause when we find a legit line to print
				gcodereader = this;
				var tokens = text.split(' ');
			    if (tokens) {
			      var cmd = tokens[0];
			      var args = {
			        'cmd': cmd
			      };
			      tokens.splice(1).forEach(function(token) {
			        var key = token[0].toLowerCase();
			        var value = parseFloat(token.substring(1));
			        args[key] = value.toFixed(5);				        
			      });

			      //calculate speed
			      var speed = Math.round(1 / (os.uptime() - TIMElast));
			      TIMElast = os.uptime();
			      //

			      console.log(linecounter + 'L ' + JSON.stringify(args));
			      NEWGODE = JSON.stringify(args);
			      
			    }
			}
        })

        .on ("end", function (){
            console.log("==== DONE ====")
            var totaltime = os.uptime() - STARTTIME;
            console.log(" Total time: " + Math.round(totaltime) + "seconds");
            console.log(" Total lines: " + linecounter + "of gcode");
            console.log(" Total speed: " + linecounter/totaltime + "lines per second average");
        })
        .read ();



var thermistorlookupcelsius = function(val) {
	/*
	Expects a voltage value between 1-1024 and returns Celsius reading.
	Component: Honeywell 135-104LAG-J01
	*/
	
	var thermistorlookup = [ 
	   {"val":1, "celsius": 500},
	   {"val":46, "celsius": 270}, //top rating 300C
	   {"val":50, "celsius": 265},
	   {"val":54, "celsius": 260},
	   {"val":58, "celsius": 255},
	   {"val":62, "celsius": 250},
	   {"val":67, "celsius": 245},
	   {"val":72, "celsius": 240},
	   {"val":79, "celsius": 235},
	   {"val":85, "celsius": 230},
	   {"val":91, "celsius": 225},
	   {"val":99, "celsius": 220},
	   {"val":107, "celsius": 215},
	   {"val":116, "celsius": 210},
	   {"val":126, "celsius": 205},
	   {"val":136, "celsius": 200},
	   {"val":149, "celsius": 195},
	   {"val":160, "celsius": 190},
	   {"val":175, "celsius": 185},
	   {"val":191, "celsius": 180},
	   {"val":209, "celsius": 175},
	   {"val":224, "celsius": 170},
	   {"val":246, "celsius": 165},
	   {"val":267, "celsius": 160},
	   {"val":293, "celsius": 155},
	   {"val":316, "celsius": 150},
	   {"val":340, "celsius": 145},
	   {"val":364, "celsius": 140},
	   {"val":396, "celsius": 135},
	   {"val":425, "celsius": 130},
	   {"val":460, "celsius": 125},
	   {"val":489, "celsius": 120},
	   {"val":526, "celsius": 115},
	   {"val":558, "celsius": 110},
	   {"val":591, "celsius": 105},
	   {"val":628, "celsius": 100},
	   {"val":660, "celsius": 95},
	   {"val":696, "celsius": 90},
	   {"val":733, "celsius": 85},
	   {"val":761, "celsius": 80},
	   {"val":794, "celsius": 75},
	   {"val":819, "celsius": 70},
	   {"val":847, "celsius": 65},
	   {"val":870, "celsius": 60},
	   {"val":892, "celsius": 55},
	   {"val":911, "celsius": 50},
	   {"val":929, "celsius": 45},
	   {"val":944, "celsius": 40},
	   {"val":959, "celsius": 35},
	   {"val":971, "celsius": 30},
	   {"val":981, "celsius": 25},
	   {"val":989, "celsius": 20},
	   {"val":994, "celsius": 15},
	   {"val":1001, "celsius": 10},
	   {"val":1005, "celsius": 5}	
	]	

	var last = thermistorlookup[0];
	var cur = thermistorlookup[0];
	
	for (var data in thermistorlookup) {
		cur = thermistorlookup[data];
		if ((cur.val > val) && (last.val <= val)) {
			var c = cur.celsius - last.celsius;
			var v = cur.val - last.val;
			var vd = val  - last.val;
			var ratio = vd/v;
			//console.log(last.celsius+(c*ratio))
			return last.celsius+(c*ratio);
		}
		last = thermistorlookup[data];
	}
}



