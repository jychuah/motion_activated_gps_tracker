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
      particle: "libs/particle.min",
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
      name: "fbaccountdropdown",
      location: "packages/fb-account-dropdown",
      main: "fb-account-dropdown"
    },
    {
      name: "pbdevicedropdown",
      location: "packages/pb-device-dropdown",
      main: "pb-device-dropdown"
    },
    {
      name: "pbcontrolpanel",
      location: "packages/pb-control-panel",
      main: "pb-control-panel"
    }
  ]
});


require(['require'], function(require) {
  require(['app'], function(App) {
      var a = new App();
  });
});
