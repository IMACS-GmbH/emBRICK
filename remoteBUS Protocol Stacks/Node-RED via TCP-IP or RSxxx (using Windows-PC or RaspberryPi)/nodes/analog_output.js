module.exports = function(RED) {
    function AnalogOutputNode(config) {
        RED.nodes.createNode(this,config);
        // Get the User Configuration Data from Node Properties
        var node = this;
        var topic = config.topic;
        var brick = config.brick;
        var byte_pos = config.byte_pos;

        var digits_out_lower = config.digits_out_lower;
        var digits_out_upper = config.digits_out_upper;

        var process_unit = config.process_unit;
        var process_out_lower = config.process_out_lower;
        var process_out_upper = config.process_out_upper;

        // Access the node's context object
        var flow = this.context().flow;
        // Create a empty Variable oldValue
        var oldValue;

        // Waiting of a Message Input
        node.on('input', function(msg) {
            var digits_in_lower = msg.digits_in_lower;
            var digits_in_upper = msg.digits_in_upper;
            var digitIn = msg.payload;
            var force = msg.force;

            var digitOut = parseInt(digits_out_lower)+((digitIn-digits_in_lower)*
            (digits_out_upper-digits_out_lower)/(digits_in_upper-digits_in_lower));
            //digitOut = Math.round(digitOut);
            // Get the Output Array from Remote Master
            var output = flow.get("Output");
            // Calculate the Position in the OutputBuffer
            var output_pos = parseInt(brick) + parseInt(byte_pos);
            // To make the Variable a Integer
            output_pos = parseInt(output_pos);
            // Create a Buffer from Size 2
            const buf = Buffer.allocUnsafe(2);
            // Fill the Buffer with 00
            buf.fill(0);
            // Write the Input Msg in the Buffer (16Bit Big Endian Format)
            buf.writeUInt16BE(parseInt(digitOut),0);
            // Add the 2 bytes(16bit) to the output Array
            output[output_pos] = buf[1];
            output[parseInt(output_pos)+1] = buf[0];
            // Update the Output Array for the Remote Master
            flow.set("Output",output);
            // set the topic as msg.topic
            msg.topic = topic;
            //
            msg.payload = digitOut;//(output[parseInt(output_pos)+1]<< 8) +output[output_pos];
            msg.process_unit = process_unit;
            // Give the State Informatian under the Node
            msg.digitOut = Math.round(digitOut);
            msg.force = force;
            msg.in_or_out = "Output";


            if(process_out_lower != "undefined" && process_out_upper >0){
              var process_value = process_out_lower + ((digitOut-digits_out_lower)*
              (process_out_upper-process_out_lower)/(digits_out_upper-digits_out_lower));
              process_value = Math.round(process_value)
              node.status({text: "AO "+process_value + " " + process_unit});
              msg.process_value = process_value;
            }
            else{
              node.status({text: "AO "+ msg.payload + " Digits", force: force});
            }

            if(oldValue != msg.payload){
              node.send(msg);
              oldValue = msg.payload;
            }
        });
    }
    RED.nodes.registerType("Analog Output",AnalogOutputNode);
}
