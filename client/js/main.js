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
      location :  "packages/pb-devices-modal",
      main: "pb-devices-modal"
    },
    {
      name: "pbloginmodal",
      location: "packages/pb-login-modal",
      main: "pb-login-modal"
    },
    {
      name: "fbloginmodal",
      location: "packages/fb-login-modal",
      main: "fb-login-modal"
    },
    {
      name: "fbchangepwmodal",
      location: "packages/fb-changepw-modal",
      main: "fb-changepw-modal"
    },
  ]
});


require(['require'], function(require) {
  require(['app'], function(App) {
      var a = new App();
  });
});
