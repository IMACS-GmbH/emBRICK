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
           <md-icon ng-if="msg.payload" style="color:gold" >circle</md-icon>
           <md-icon ng-if="!(msg.payload)" style="color:black" >circle</md-icon>
         </div>
        </md-content>
        `;
        return html;
    }
    function checkConfig(node, conf) {
        if (!conf || !conf.hasOwnProperty("group")) {
            node.error(RED._("ui_digital-vis.error.no-group"));
            return false;
        }
        return true;
    }

    var ui = undefined;

    function DI_VIS_Node(config) {
        try {
            if(ui === undefined) {
                ui = RED.require("node-red-dashboard")(RED);
            }
            RED.nodes.createNode(this, config);
                var node = this;
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

                        $scope.init = function (config) {
                            $scope.config = config;
                            $scope.name = config.name || "Digital Vis";
                        };

                        $scope.$watch('msg', function(msg){
                          $scope.topic = msg.topic;
                          if(msg.payload === true){
                            $scope.payload = 1;
                          }
                          else if(msg.payload === false){
                            $scope.payload = 0;
                          }
                          else{
                            $scope.payload = msg.payload;
                          }
                          $scope.send({topic: $scope.name, payload: $scope.payload, in_or_out: "Output"});

                        });
                      }
                    });
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
    RED.nodes.registerType("ui_digital-vis", DI_VIS_Node);
  })
}
