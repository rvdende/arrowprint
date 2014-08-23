// Arrow command line relay
// this program scans for connected devices and relays communication to a socket server
// v 5.4beta


var file = "";

if (!process.argv[2]) {
	console.log("\n\nERROR. No gcode file.\n Please specify a gcode file to print.")
	console.log("node arrowprint.js yourpart.gcode\n\n")
} else {
	file = process.argv[2];
}


var gcodecached = [];

var arduino = {};
var SerialPort = require("serialport"); //so we can access the serial port
SerialPort.list( function (err, ports) {
	console.log("Found devices:")
	ports.forEach( function(port) {
		console.log('PORT:'+port.comName);
		console.log('ID:'+port.pnpId);		
		console.log(port.manufacturer+"\n");

		if (port.pnpId.indexOf('duino') > 0) {
			console.log('FOUND ARDUINO')
			//var arduino = new SerialPort.SerialPort('/dev/serial/by-id/'+port.pnpId, {baudrate: 115200});			

		} else {
			//console.log('scanning...');					
		}		
	});

	arduino = new SerialPort.SerialPort('COM21', {baudrate: 115200});			
	
});


console.log("\nLoading "+file+" ... please wait\n")

var os = require("os");
var TIMElast = os.uptime();
var STARTTIME = os.uptime();
var reader = require ("buffered-reader");
var BinaryReader = reader.BinaryReader;
var DataReader = reader.DataReader;
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

			      //calculate movement speed from direction/momentum etc.

			      //calculate communication speed
			      var speed = Math.round(1 / (os.uptime() - TIMElast));
			      TIMElast = os.uptime();
			      //

			      //console.log(linecounter + 'L ' + JSON.stringify(args));
			      //if (Math.random() < 0.01) { process.stdout.write("."); }
			      NEWGODE = JSON.stringify(args);
			      gcodecached.push(NEWGODE);			      
			    }
			}
        })

        .on ("end", function (){
            console.log("end")
            var totaltime = os.uptime() - STARTTIME;
            console.log(" Total time: " + Math.round(totaltime) + "seconds");            
            console.log(" Total lines: " + linecounter + "of gcode");
            console.log(" Total speed: " + linecounter/totaltime + "lines per second average");
			console.log(gcodecached.length)
            //console.log(gcodecached)

            console.log("attempting to print..")
            gcodeprint(arduino);
        })
        .read ();

var gcodeindex = 0;
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
			if (gcodecached.length > gcodeindex) {
				arduino.write(gcodecached[gcodeindex]); //writes to arduino
				console.log(gcodecached[gcodeindex]);
				gcodeindex++;
			}
		}

		if (obj.thermistor) {
			//var temperature = (obj.thermistor/Math.pow(2,16))*1024;
			//temperature = thermistorlookupcelsius(temperature);
			//console.log("Thermistor " + obj.thermistor);

		}

	});
}