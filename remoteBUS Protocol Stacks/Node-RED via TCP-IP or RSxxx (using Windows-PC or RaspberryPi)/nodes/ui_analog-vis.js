module.exports = function(RED) {
  function HTML(config) {
    var configAsJson = JSON.stringify(config);
    var html = String.raw`
     <md-content ng-init='init(` + configAsJson + `)' layout-align="space-between stretch" layout='row'>
      <div>
        <h4>{{name}}  {{msg.in_or_out}}</h4>
        <p></p>
      </div>
      <div>
        <h4 ng-if="visible"> {{val}} Digits </h4>
        <h4> {{pro_val}} {{unit}} </h4>
        <p> </p>
      </div>
    </md-content>
    `;
    return html;
  }

  function checkConfig(node, conf) {
      if (!conf || !conf.hasOwnProperty("group")) {
          node.error(RED._("ui_analog-vis.error.no-group"));
          return false;
      }
      return true;
  }

  var ui = undefined;

  function AI_VIS_Node(config) {
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
                      $scope.counter = 0;
                      $scope.val;
                      $scope.visible = false;


                      $scope.init = function (config) {
                        $scope.config = config;
                        $scope.name = config.name || "Analog Vis";
                      };

                      $scope.$watch('msg', function(msg) {
                        if (msg.force){
                          $scope.val=msg.payload;
                        }
                        $scope.unit = msg.process_unit;
                        if(msg.process_value === undefined){
                          $scope.visible = true;
                        }
                        if($scope.counter){
                          $scope.val = msg.payload;
                          $scope.pro_val = msg.process_value;
                        }
                        $scope.counter +=1;
                        if ($scope.counter === 500){

                          $scope.counter = 0;
                        }
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
        RED.nodes.registerType("ui_analog-vis", AI_VIS_Node);
    })
}
