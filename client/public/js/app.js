define(['jquery',
        'particle',
        'UI',
        'bootstrapgrowl',
        'bootstrap'], function($, Particle) {
  function App() {
      this.particle = new Particle();
      $(document).ready(this.initialize.apply(this));
  };
  App.prototype = {
      constructor: App,

      deviceSelectCallback: function(device) {
        console.log("Currently selected device", device);
        console.log("User profile", this.ui.profile);
      },

      initialize: function() {
        this.ui = new UI($.proxy(this.deviceSelectCallback, this));
      }
  };
  return App;
});
