define('particlefunctionpanel',
  ['jquery', 'particle',
    'text!./html/particle-function-panel.html',
    'text!./html/particle-function-input-group.html',
    'particlebase',
    'bootstrapgrowl'],
  function($, Particle, panelHtml, inputGroupHtml) {
  ParticleFunctionPanel = function(device_id, access_token) {
    var token = access_token;
    var particle = new Particle();
    var id = device_id
    this.$particlefunctionpanel = $('.particle-function-panel');

    function callFunction(eventSource) {
      var group = $(eventSource.target).parents('[particle-function-input-group="group"]');
      var functionname = group.find('[particle-function-input-group="function"]').html();
      var param = group.find('[particle-function-input-group="parameter"]').val();
      var fnPr = particle.callFunction({ deviceId: id, name: functionname, argument: param, auth: token});
      fnPr.then(
        function(data) {
          $.bootstrapGrowl(functionname + " returned " + data.body.return_value, { type : "success"} );
        },
        function(error) {
          $.bootstrapGrowl(functionname + " call was unsuccessful", { type : "warning"} );
        }
      );
    }

    function probeDevice() {
      clearPanel.apply(this);
      var devicesPr = particle.getDevice({ deviceId: id, auth: token});
      devicesPr.then(
        $.proxy(function(data) {
          if (data.body.functions) {
            var panel = this.$particlefunctionpanel.find('[particle-function-panel="function_list"]');
            var functions = data.body.functions;
            this.$particlefunctionpanel.find('[particle-function-panel="status"]').html(functions.length + " functions found");
            for (var key in functions) {
              var inputGroup = $($(inputGroupHtml));
              inputGroup.find('[particle-function-input-group="function"]').html(functions[key]);
              inputGroup.find('[particle-function-input-group="call"]').click(callFunction);
              panel.append(inputGroup);
            }
          } else {
            this.$particlefunctionpanel.find('[particle-function-panel="status"]').html("No functions");
          }
        }, this),
        $.proxy(function(error) {

        }, this)
      );
    }

    function clearPanel() {
      var panel = this.$particlefunctionpanel.find('[particle-function-panel="function_list"]');
      panel.html("");
    }

    function init() {
      this.$particlefunctionpanel.html(panelHtml);
      probeDevice.apply(this);
      var ref = this;

      particle.getEventStream({ deviceId: id, auth: token }).then(
        function(stream) {
          stream.on('event', function(data) {
            if (data.data === "online") {
              probeDevice.apply(ref);
            }
            if (data.data === "offline") {
              clearPanel.apply(ref);
              ref.$particlefunctionpanel.find('[particle-function-panel="status"]').html("Offline");
            }
          });
        }
      );

    }

    $('.particle-function-panel').addClass('panel panel-default');
    init.apply(this);
  }

  ParticleFunctionPanel.prototype = {
    constructor: ParticleFunctionPanel,
  }
  return ParticleFunctionPanel;
});
