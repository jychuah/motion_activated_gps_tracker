define('particlevariablepanel',
  ['jquery', 'particle',
    'text!particlevariablepanel/html/particle-variable-panel.html',
    'text!particlevariablepanel/html/particle-variable-input-group.html',
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
          console.log("Variable: ", data.body.result);
        },
        function(error) {
          console.log("Error: ", error);
        }
      );
    }

    function init() {
      this.$particlevariablepanel.html(panelHtml);
      var devicesPr = particle.getDevice({ deviceId: id, auth: token});
      devicesPr.then(
        $.proxy(function(data) {
          if (data.body.variables) {
            var panel = this.$particlevariablepanel.find('[particle-variable-panel="variable_list"]');
            var variables = data.body.variables;
            for (var key in variables) {
              var inputGroup = $($(inputGroupHtml));
              inputGroup.find('[particle-variable-input-group="variable"]').html(key);
              inputGroup.find('[particle-variable-input-group="refresh"]').click(refreshVariable);
              panel.append(inputGroup);
            }
          }
        }, this),
        $.proxy(function(error) {

        }, this)
      );
      /*
      this.$particlevariablepanel.find('[particle-variable-panel="get_variable_button"]').click(
        $.proxy(function(eventSource) {
          particle.getVariable({ deviceId: id, name: 'countVar', auth: token }).then(function(data) {
          console.log('Device variable retrieved successfully:', data);
        }, function(err) {
          console.log('An error occurred while getting attrs:', err);
        });
        }, this)
      );
      var devicesPr = particle.getDevice({ deviceId: id, auth: token});
      devicesPr.then(function(data) {
        console.log('Device attrs: ', data);
      },
      function(error) {
        console.log('API failed: ', error);
      });
      */

    }

    $('.particle-variable-panel').addClass('panel panel-default');
    init.apply(this);
  }

  ParticleVariablePanel.prototype = {
    constructor: ParticleVariablePanel,
  }
  return ParticleVariablePanel;
});
