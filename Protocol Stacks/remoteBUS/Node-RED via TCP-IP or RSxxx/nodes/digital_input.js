module.exports = function(RED){
    function DigitalInputNode(config)
    {
        RED.nodes.createNode(this,config);
        // Get the User Configuration Data from Node Properties
        var node = this;
        var brick = config.brick;
    		var byte_pos = config.byte_pos;
        var bit_pos = config.bit_pos;
        var id = "DI: " + brick + ", " + byte_pos + ", " + bit_pos;
    		var topic = config.topic || id;

        var debounce = config.debounce || 5;

        // Access the node's context object
        var flow = this.context().flow;
        // Create a emptpy Message variable
        var msg ={};
        // Create a Variable to save the old msg.payload
        var oldValue;
        // Create Variable to save the bit State
        var bit;
        // get the User configurated updateRate from the flow
        var updateRate = flow.get("updateRate");
        // Create a Variable for the debounce
        var debouncer;

        // Create the Function get for the Interval Loop
        function get(){
          // Get the Preconfigured Variable sys_approved to check if the connecting to
          // the Remote Master is avaible (0 not connected) (1 connected)
          var sys_approved = flow.get("sys_approved");
          // if sys_approved = 0 also Remote Master still not connected
          if (sys_approved == 0){
            // Give the State Informatian under the Node
            node.status({fill:"red",shape:"ring",text:"Remote Master not ready"});
          }
          else if(sys_approved == 1){
            // Get the Brick Inputs from Remote Master
            var brick_g = flow.get("Brick_Input");
            //Take the actually Value of the wished Brick -> Byte position
            var brick_input = brick_g[brick][byte_pos];
            // Take the Value of the wished Bit Position (1 or 0)
            bit = (brick_input>>bit_pos)%2;
            // Give the Value as a Int value back
            bit = parseInt(bit)
            // if Bit = 1
            if(bit){
              // A if loop to interrupt a endless Message Output
              // oldValue(oldBitValue) is not same as bit (actually Bit)
              if(oldValue != bit){
                // Save the Bit in the oldValue
                oldValue = bit;
                // Give the State Informatian under the Node
                node.status({fill:"yellow",shape:"dot",text:bit});
                // Create a Timeout function to debounce
                // only if the Bit is still 1 after the timeout send it as msg
                debouncer = setTimeout(function getBit(){
                  // Send a msg on the node Output
                  node.send({payload: bit, topic: topic, in_or_out: "Input"});
                }, debounce)
              }

            }
            // ekse Bit = 0
            else{
              // oldValue(oldBitValue) is not same as bit (actually Bit)
              if(oldValue != bit){
                // Save the Bit in the oldValue
                  oldValue = bit;
                  // Give the State Informatian under the Node
                  node.status({fill:"black",shape:"dot",text:bit});
                  // Send a msg on the node Output
                  node.send({payload: bit, topic: topic, in_or_out: "Input"});
              }

            }
          }
      }
      // Set a Interval to get always the actual Value from Remote Master
      // in the Configured updateRate
      time_id = setInterval(get,updateRate);
    }
    RED.nodes.registerType("Digital Input",DigitalInputNode);
}
