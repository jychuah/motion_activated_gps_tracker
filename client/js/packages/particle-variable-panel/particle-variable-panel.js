define('particlevariablepanel',
  ['jquery', 'particle',
    'text!./html/particle-variable-panel.html',
    'text!./html/particle-variable-input-group.html',
    'particlebase',
    'bootstrapgrowl'],
  function($, Particle, panelHtml, inputGroupHtml) {
  ParticleVariablePanel = function(device_id, access_token) {
    var token = access_token;
    var particle = new Particle();
    var id = device_id
    this.$particlevariablepanel = $('.particle-variable-panel');

    function refreshVariable(eventSource) {
      var group = $(eventSource.target).parents('[particle-variable-input-group="group"]');
      var varname = group.find('[particle-variable-input-group="variable"]').html();
      var value = group.find('[particle-variable-input-group="value"]');
      particle.getVariable({ deviceId: id, name: varname, auth: token}).then(
        function(data) {
          value.val(data.body.result);
        },
        function(error) {
          $.bootstrapGrowl("Unable to refresh " + varname, { type : "warning"} );
        }
      );
    }

    function probeDevice() {
      clearPanel.apply(this);
      var devicesPr = particle.getDevice({ deviceId: id, auth: token});
      devicesPr.then(
        $.proxy(function(data) {
          if (data.body.variables) {
            var panel = this.$particlevariablepanel.find('[particle-variable-panel="variable_list"]');
            var variables = data.body.variables;
            var keycount = 0;
            for (var key in variables) {
              var inputGroup = $($(inputGroupHtml));
              inputGroup.find('[particle-variable-input-group="variable"]').html(key);
              inputGroup.find('[particle-variable-input-group="refresh"]').click(refreshVariable);
              panel.append(inputGroup);
              keycount++;
            }
            this.$particlevariablepanel.find('[particle-variable-panel="status"]').html(keycount + " variables found");

          } else {
            this.$particlevariablepanel.find('[particle-variable-panel="status"]').html("No variables");
          }
        }, this),
        $.proxy(function(error) {

        }, this)
      );
    }

    function clearPanel() {
      var panel = this.$particlevariablepanel.find('[particle-variable-panel="variable_list"]');
      panel.html("");
    }

    function init() {
      this.$particlevariablepanel.html(panelHtml);
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
              ref.$particlevariablepanel.find('[particle-variable-panel="status"]').html("Offline");
            }
          });
        }
      );
    }

    $('.particle-variable-panel').addClass('panel panel-default');
    init.apply(this);
  }

  ParticleVariablePanel.prototype = {
    constructor: ParticleVariablePanel,
  }
  return ParticleVariablePanel;
});
