module.exports = function(RED) {
  function HTML(config) {
    var configAsJson = JSON.stringify(config);
    var html = String.raw`
    <md-content ng-init='init(` + configAsJson + `)' layout-align="space-between stretch" layout='row'>
      <md-switch ng-model="force">
        {{name}}
      </md-switch>
      <md-input-container ng-if="force" class="md-block">
        <label>Force</label>
        <input ng-model="forcing" ng-model-options="{updateOn : 'change'}"  ng-change="forcen(forcing)" placeholder="0-65535">
      </md-input-container>
    </md-content>
    `;
    return html;
  }

  function checkConfig(node, conf) {
      if (!conf || !conf.hasOwnProperty("group")) {
          node.error(RED._("ui_analog-force.error.no-group"));
          return false;
      }
      return true;
  }

  var ui = undefined;

  function ANALOG_FORCE_Node(config) {
      try {
          var node = this;
          if(ui === undefined) {
              ui = RED.require("node-red-dashboard")(RED);
          }
          RED.nodes.createNode(this, config);

          if (checkConfig(node, config)) {
              var html = HTML(config);                    // *REQUIRED* !!DO NOT EDIT!!
              var done = ui.addWidget({                   // *REQUIRED* !!DO NOT EDIT!!
                  node: node,                             // *REQUIRED* !!DO NOT EDIT!!
                  order: config.order,                    // *REQUIRED* !!DO NOT EDIT!!
                  group: config.group,                    // *REQUIRED* !!DO NOT EDIT!!
                  width: config.width,                    // *REQUIRED* !!DO NOT EDIT!!
                  height: config.height,                  // *REQUIRED* !!DO NOT EDIT!!
                  format: html,                           // *REQUIRED* !!DO NOT EDIT!!
                  templateScope: "local",                 // *REQUIRED* !!DO NOT EDIT!!
                  emitOnlyNewValues: false,               // *REQUIRED* Edit this if you would like your node to only emit new values.
                  forwardInputMessages: false,            // *REQUIRED* Edit this if you would like your node to forward the input message to it's ouput.
                  storeFrontEndInputAsState: false,       // *REQUIRED* If the widgect accepts user input - should it update the backend stored state ?

                  convertBack: function (value) {
                      return value;
                  },


                  beforeEmit: function(msg, value) {
                      return { msg: msg};
                  },

                  beforeSend: function (msg, orig) {
                      if (orig) {
                          return orig.msg;
                      }
                  },

                  initController: function($scope, events) {
                      //debugger;
                      $scope.flag = true;   // not sure if this is needed?
                      $scope.force = false;


                      $scope.init = function (config) {
                        $scope.config = config;
                        $scope.name = config.name || "Analog Force";

                      };

                      $scope.$watch('msg', function(msg) {
                        if(msg.in_or_out == "Input"){
                          $scope.in_or_out = "Output";
                        }
                        else{
                          $scope.in_our_out = "Input";
                        }
                        $scope.payload = msg.payload;
                        $scope.bool_force = msg.force;
                        $scope.digits_in_lower = msg.digits_in_lower;
                        $scope.digits_in_upper = msg.digits_in_upper;
                        $scope.digits_out_lower = msg.digits_out_lower;
                        $scope.digits_out_upper = msg.digits_out_upper;
                        $scope.process_value = msg.process_value;
                        //$scope.send({topic: $scope.name, payload: msg.payload, output: "Output"})
                        if(!($scope.force)){
                          $scope.send({topic: $scope.name, payload: $scope.payload,
                            in_or_out: $scope.in_our_out, process_value: $scope.process_value,
                          digits_in_lower: $scope.digits_in_lower, digits_in_upper: $scope.digits_in_upper, force: $scope.bool_force});
                        }
                        if($scope.force){
                          if($scope.forcing!= val){
                            $scope.forcing = val;
                            $scope.send({topic: $scope.name, payload: $scope.forcing,
                              in_or_out: $scope.in_our_out, process_value: $scope.process_value,
                            digits_in_lower: $scope.digits_in_lower, digits_in_upper: $scope.digits_in_upper, force: true});

                          }
                        }
                        $scope.forcen = function(val){
                          if($scope.forcing!= val){
                            $scope.forcing = val;
                            $scope.send({topic: $scope.name, payload: $scope.forcing,
                              in_or_out: $scope.in_our_out, process_value: $scope.process_value,
                            digits_in_lower: $scope.digits_in_lower, digits_in_upper: $scope.digits_in_upper, force: true});

                          }
                        };
                      });

                    }
                });
            }
        }
        catch (e) {
            // eslint-disable-next-line no-console
            console.warn(e);		// catch any errors that may occur and display them in the web browsers console
        }

        node.on("close", function() {
            if (done) {
                done();
            }
        });
/*******************************************************************/
    }


    setImmediate(function() {
        RED.nodes.registerType("ui_analog-force", ANALOG_FORCE_Node);
    })
}
