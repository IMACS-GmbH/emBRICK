module.exports = function(RED) {
  function HTML(config) {
    var configAsJson = JSON.stringify(config);
    var html = String.raw`
    <md-content ng-init='init(` + configAsJson + `)' layout-align="space-between stretch" layout='row'>
      <md-select ng-model="force" ng-model=farbe placeholder="{{name}}" class="md-no-underline" ng-change="change()">
        <md-option >true</md-option>
        <md-option >false</md-option>
        <md-option >off</md-option>
      </md-select>

      <div layout='row'>
        <md-icon style="color:{{farbe}}" ng-if=options>bolt</md-icon>
      </div>
    </md-content>
    `;
    return html;
  }
  function checkConfig(node, conf) {
      if (!conf || !conf.hasOwnProperty("group")) {
          node.error(RED._("ui_digital-force.error.no-group"));
          return false;
      }
      return true;
  }
  var ui = undefined;

  function DIGITAL_FORCE_Node(config) {
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
                      $scope.value = "off";
                      //$scope.farbe = "black";
                      $scope.options = false;

                      $scope.force = "off";


                      $scope.init = function (config) {
                        $scope.config = config;
                        $scope.name = config.name || "Digtal Force";

                      };

                      $scope.$watch('msg', function(msg) {
                        $scope.topic = msg.topic;
                        if(msg.in_or_out == "Input"){
                          $scope.in_or_out = "Output";
                        }
                        else{
                          $scope.in_our_out = "Input";
                        }
                        if(msg.payload === true){
                          $scope.payload = 1;
                        }
                        else if(msg.payload === false){
                          $scope.payload = 0;
                        }
                        else{
                          $scope.payload = msg.payload;
                        }

                        if($scope.force == "off"){

                          $scope.send({topic: $scope.name, payload: $scope.payload, in_or_out: $scope.in_or_out, force: $scope.force});
                        }


                      });
                      $scope.change = function(){
                        if($scope.force == "true"){
                          $scope.farbe = "gold";
                          $scope.options = true;
                          $scope.send({topic: $scope.name, payload: 1, in_or_out: $scope.in_or_out});
                        }
                        else if($scope.force == "false"){
                          $scope.farbe = "black";
                          $scope.options = true;
                          $scope.send({topic: $scope.name, payload: 0, in_or_out: $scope.in_or_out});
                        }
                        else{
                          $scope.options = false;
                          $scope.send({topic: $scope.name, payload: $scope.payload, in_or_out: $scope.in_or_out})
                        }
                      }

                      $scope.config = function(){
                          $scope.options = !scope.options;
                      };

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
        RED.nodes.registerType("ui_digital-force", DIGITAL_FORCE_Node);
    })
}
