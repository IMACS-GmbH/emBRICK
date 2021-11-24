module.exports = function(RED) {
    function DigitalOutputNode(config) {
        RED.nodes.createNode(this,config);
        // Get the User Configuration Data from Node Properties
        var node = this;
        var brick = config.brick;
        var byte_pos = config.byte_pos;
        var bit_pos = config.bit_pos;
        var id = "DO: " + brick + ", " + byte_pos + ", " + bit_pos;
        var topic = config.topic || id;

        //var val = config.val;
        var flow = this.context().flow;
        // Create a Variable to save the old msg.payload
        var oldValue;
        // Waiting of a Message Input
        node.on('input', function(msg) {
          msg.in_or_out = "Output";
          // Calculate the Position in the OutputBuffer
          var output_pos = parseInt(brick)+parseInt(byte_pos);
          // Calculate the Value from binary to decimal
          var bit_val = 1 << bit_pos;
          // Check if msg.payload is 1 and msg.payload is not equal to oldValue
          if((msg.payload === 1 || msg.payload == true) && msg.payload != oldValue){
            // Save msg.payload in the Variable oldValue
            oldValue = msg.payload;
            // Get the Output Array from Remote Master
            var output = flow.get("Output");
            // Take the value from Output Array
            var value_out_pos = output[output_pos];
            // Give the State Informatian under the Node
            node.status({fill:"yellow",shape:"dot",text:msg.payload});
            // Check if value_out_pos is not 1
            if(!(value_out_pos>>bit_pos)){
              // Add the bit_val to value_out_pos and put it in the Output Array
              output[output_pos] = value_out_pos + bit_val;
              // Update the Output Array for the Remote Master
              flow.set("Output",output);
              node.send(msg);

              }
            }
            // Check if msg.payload is 0 and msg.payload is not equal to oldValue
            else if((msg.payload === 0 || msg.payload == false) && msg.payload != oldValue){
              // Save msg.payload in the Variable oldValue
              oldValue = msg.payload;
              // Get the Output Array from Remote Master
              var output = flow.get("Output");
              // Take the value from Output Array
              var value_out_pos = output[output_pos];
              // Give the State Informatian under the Node
              node.status({fill:"black",shape:"ring",text:msg.payload});
              // Check if value_out_pos is 1
              if(value_out_pos>>bit_pos){
                // Remove the bit_val to value_out_pos and put it in the Output Array
                output[output_pos] = value_out_pos - bit_val;
                // Update the Output Array for the Remote Master
                flow.set("Output",output);
                node.send(msg);
              }
            }

        });
    }
    RED.nodes.registerType("Digital Output",DigitalOutputNode);
}
