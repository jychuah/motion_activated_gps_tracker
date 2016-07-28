require.config({
  baseUrl: "js",
  shim : {
        "bootstrap" : { "deps" :['jquery'] },
        "bootstrapgrowl" : { "deps" :['jquery', 'bootstrap'] }
    },
  paths: {
      jquery: "libs/jquery-2.2.0.min",
      app: "app",
      "bootstrap" :  "libs/bootstrap.min",
      text: "libs/text",
      particle: "libs/particle.min",
      particlebase: "libs/ParticleBase",
      bootstrapgrowl : "libs/jquery.bootstrap-growl.min"
  },
  packages: [
    {
      name: "particleloginmodal",
      location: "packages/particle-login-modal",
      main: "particle-login-modal"
    }
  ]
});


require(['require'], function(require) {
  require(['app'], function(App) {
      var a = new App();
  });
});
