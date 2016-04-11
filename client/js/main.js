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
      text: "libs/text",
      bootstrapgrowl : "libs/jquery.bootstrap-growl.min"
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
    {
      name: "fbcreatemodal",
      location: "packages/fb-create-modal",
      main: "fb-create-modal"
    },
    {
      name: "fbresetmodal",
      location: "packages/fb-reset-modal",
      main: "fb-reset-modal"
    },
    {
      name: "fbaccountnavbar",
      location: "packages/fb-account-navbar",
      main: "fb-account-navbar"
    }
  ]
});


require(['require'], function(require) {
  require(['app'], function(App) {
      var a = new App();
  });
});
