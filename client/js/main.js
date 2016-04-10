require.config({
  baseUrl: "js",
  shim : {
        "bootstrap" : { "deps" :['jquery'] },
        "firebase" : { exports: 'Firebase' }
    },
  paths: {
      jquery: "libs/jquery-2.2.0.min",
      particlebase: "libs/ParticleBase",
      app: "app",
      "bootstrap" :  "libs/bootstrap.min",
      firebase : "libs/firebase",
      text: "libs/text"
  },
  packages: [
    {
      name: "pbdevicesmodal",
      location :  "libs/pb-devices-modal",
      main: "pb-devices-modal"
    },
    {
      name: "pbloginmodal",
      location: "libs/pb-login-modal",
      main: "pb-login-modal"
    }
  ]
});


require(['require'], function(require) {
  require(['app'], function(App) {
      var a = new App();
  });
});
