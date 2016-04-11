define('fbaccountnavbar',
  ['jquery',
    'text!fbaccountnavbar/html/fb-account-navbar.html',
    'firebase', 'bootstrap',
    'fbloginmodal', 'fbchangepwmodal',
    'fbcreatemodal', 'fbresetmodal',
    'bootstrapgrowl'],
  function($, navbarHtml) {
  FBAccountNavbar = function(firebase) {
    var fb = firebase;
    this.$fbaccountnavbar = $('.fb-account-navbar');

    function fb_logout() {
      fb.unauth();
    }

    function init() {
      this.$fbaccountnavbar.html(navbarHtml);
      this.fbloginmodal = new FBLoginModal(fb, function(error, auth) {
        if (error) {
          $.bootstrapGrowl("Couldn't log in", { type : 'warning' });
        } else {
          $.bootstrapGrowl("Successfully logged in", { type : 'success' });
        }
      });
      this.fbchangepwmodal = new FBChangePwModal(fb, function(error) {
        if (error) {
          $.bootstrapGrowl("Password could not be changed", { type : 'warning' });
        } else {
          $.bootstrapGrowl("Password changed", { type : 'success' });
        }
      });
      this.fbcreatemodal = new FBCreateModal(fb, function(error, authData) {
        if (error) {
          $.bootstrapGrowl("Could not create new user", { type : 'warning' });
        } else {
          $.bootstrapGrowl("User created!", { type : 'success' });
        }
      });
      this.fbresetmodal = new FBResetModal(fb, function(error) {
        if (error) {
          $.bootstrapGrowl("Unable to send password reset email", { type : 'warning' });
        } else {
          $.bootstrapGrowl("Password reset email sent", { type : 'success' });
        }
      });
      $("#fb-logout-button").click(fb_logout);

      fb.onAuth(function(authData) {
        if (authData) {
          $("#sign_in_text").text(authData.password.email);
          $("#fb-login-button").addClass("hidden");
          $("#fb-changepw-button").removeClass("hidden");
          $("#fb-create-button").addClass("hidden");
          $("#fb-reset-button").removeClass("hidden");
          $("#fb-logout-button").removeClass("hidden");
        } else {
          $.bootstrapGrowl("Logged out", { type : 'success'} );
          $("#sign_in_text").text("Sign in!");
          $("#fb-login-button").removeClass("hidden");
          $("#fb-changepw-button").addClass("hidden");
          $("#fb-create-button").removeClass("hidden");
          $("#fb-reset-button").addClass("hidden");
          $("#fb-logout-button").addClass("hidden");
        }
      });
    }

    $('.fb-account-navbar').addClass('collapse navbar-collapse')
    init.apply(this);
  }

  FBAccountNavbar.prototype = {
    constructor: FBAccountNavbar,
  }
  return FBAccountNavbar;
});
