module.exports = function(RED) {
    function AnalogInputNode(config) {
        RED.nodes.createNode(this,config);
        // Get the User Configuration Data from Node Properties
        var node = this;
        var brick = config.brick;
    		var byte_pos = config.byte_pos;
        var id = "AI: " + brick + ", " + byte_pos;
        var topic = config.topic || id;

        var gleit_mw = config.gleit_mw || 0;
        var msghystere = config.msghystere || 0;

        var digits_in_lower = config.digits_in_lower;
        var digits_in_upper = config.digits_in_upper;

        var process_unit = config.process_unit;
        var process_in_lower = config.process_in_lower;
        var process_in_upper = config.process_in_upper;
        // Access the node's context object
        var flow = this.context().flow;
        // Create a emptpy Message variable
        var msg ={};

        //gleitender Mittelwert
        // Copy the Configured gleit_MW to gleitMW
        var gleitMW = gleit_mw;
        // Create a empty Variable mw_sum & mw_anzahl
        var mw_sum;
        var mw_anzahl;

        // Messagehystere
        // Create hyst_max & hyst_min give him the value 0
        var hyst_max = 0;
        var hyst_min = 0;
        // Create a empty Variable oldValue
        var oldValue;
        // get the User configurated updateRate from the flow
        var updateRate = flow.get("updateRate");


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
          // if sys_approved = 1 the connecting to the Remote Master standing
          else if(sys_approved == 1){
            // Get the Brick Inputs Array from Remote Master
            var brick_g = flow.get("Brick_Input");
            //Take the actually Value of the wished Brick -> Byte position
            var input1 = brick_g[brick][byte_pos];
            // The given BytePos+1 because a Analog Input has 16 Bit(2Byte) Inputs
            var byte_pos2 = parseInt(byte_pos) +1;
            //Take the actually Value of the wished Brick -> Byte position +1
            var input2 = brick_g[brick][byte_pos2];
            // Save the BytePos & BytePos+1 together in a Variable
            var brick_input = (input1 << 8) + input2;
            // Gleitender Mittelwert and Messagerhystere is not configurated
            if (gleit_mw == 0 && msghystere == 0){
                // Split the Information in the Messagehystere
                // msg.topic send the Userconfigured Topic or the Preconfigured Topic
                msg.topic = topic;
                // msg.payload is the Main Message it comprise actual state of Analog Input
                msg.payload = brick_input;
                // msg.input is for the VisCond to give out the Information that the Value
                // is from Input to Dashboard

                if(process_in_upper > 0 && process_in_lower != "undefined"){
                  var process_value = process_in_lower+((brick_input-digits_in_lower)*
                  (process_in_upper-process_in_lower)/(digits_in_upper-digits_in_lower));
                  process_value = Math.round(process_value)
                  node.status({text:"AI "+process_value + " " + process_unit});
                }
                else{
                  node.status({text:"AI "+brick_input + "Digits"});
                }

                msg.in_or_out = "Input";
                msg.digits_in_lower = digits_in_lower;
                msg.digits_in_upper = digits_in_upper;
                msg.process_in_lower = process_in_lower;
                msg.process_in_upper = process_in_upper;
                msg.process_value = process_value;
                msg.process_unit = process_unit;
                // Give the State Informatian under the Node

                // Send's the message
                node.send(msg);
            }
            // if Gleitender Mittelwert is Configured and is > 0 && Messagehystere is not configured or is set to 0
            else if(gleit_mw > 0 && msghystere == 0){
              // If the Value of GleitMW > 0
              if(gleitMW){
                // add the actually Brick_Input in mw_sum
                mw_sum += brick_input;
                // Every time brick_input will be added to mw_sum
                // mw_anzahl increase by 1
                mw_anzahl += 1;
                // Every time brick_input will be added to mw_sum
                //  gleitMw decrease by 1
                gleitMW -=1;
              }
              // Then when gleitMW is 0
              else if(gleitMW == 0){
                // Divide mw_sum with mw_anzahl
                let val = mw_sum / mw_anzahl;
                // Give the State Informatian under the Node
                node.status({text:"AI "+val +" Digits"});
                // Send's the message
                node.send({topic: topic, payload: val, input: "Input",
                digits_in_lower: digits_in_lower, digits_in_upper: digits_in_upper,
              process_in_lower: process_in_lower, process_in_upper: process_in_upper});
                // Set mw_sum to 0 again
                mw_sum = 0;
                // Set mw_anzahl to 0 again
                mw_anzahl = 0;
                // Give gleitMW the Configured Value of gleit_mw
                gleitMW = gleit_mw;
              }
            }
            // If Messagerhystere is Configured and is > 0 && Gleitender Mittelwert is not configured or is set to 0
            else if(msghystere > 0 && gleit_mw == 0){
              // check if oldValue is unused (undefined)
              if (oldValue === undefined){
                // Give oldValue is Value of brick_input
                oldValue = brick_input;
                // Calculate the Maximum of the hysteresis
                hyst_max = parseInt(oldValue) + parseInt(msghystere);
                // Calculate the Minimum of the hysteresis
                hyst_min = parseInt(oldValue) - parseInt(msghystere);
                // Send's the message
                node.send({topic: topic, payload: oldValue,  input: "Input",
                digits_in_lower: digits_in_lower, digits_in_upper: digits_in_upper,
              process_in_lower: process_in_lower, process_in_upper: process_in_upper})
              }
              // check if brick_input is bigger than hyst_max or smaller than hyst_min
              else if(brick_input > hyst_max || brick_input < hyst_min){
                // Give oldValue is Value of brick_input
                oldValue = brick_input;
                // Recalculate the Maximum of the hysteresis
                hyst_max = parseInt(oldValue) + parseInt(msghystere);
                // Recalculate the Minium of the hysteresis
                hyst_min = parseInt(oldValue) - parseInt(msghystere);
                // Give the State Informatian under the Node
                node.status({text:"AI "+brick_input + " Digits"});
                // Send's the message
                node.send({topic: topic, payload: oldValue,  input: "Input",
                digits_in_lower: digits_in_lower, digits_in_upper: digits_in_upper,
              process_in_lower: process_in_lower, process_in_upper: process_in_upper});
              }
            }
          }
        }
        // Set a Interval to get always the actual Value from Remote Master
        // in the Configured updateRate
        time_id = setInterval(get,updateRate);
    }
    RED.nodes.registerType("Analog Input",AnalogInputNode);
}
