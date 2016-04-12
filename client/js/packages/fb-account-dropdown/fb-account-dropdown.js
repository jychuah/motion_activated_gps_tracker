define('fbaccountdropdown',
  ['jquery',
    'text!fbaccountdropdown/html/fb-account-dropdown.html',
    'firebase', 'bootstrap',
    'fbloginmodal', 'fbchangepwmodal',
    'fbcreatemodal', 'fbresetmodal',
    'bootstrapgrowl'],
  function($, dropdownHtml) {
  FBAccountDropdown = function(firebase) {
    var fb = firebase;
    this.$fbaccountdropdown = $('.fb-account-dropdown');

    function fb_logout() {
      fb.unauth();
    }

    function init() {
      this.$fbaccountdropdown.html(dropdownHtml);
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

    $('.fb-account-dropdown').addClass('dropdown')
    init.apply(this);
  }

  FBAccountDropdown.prototype = {
    constructor: FBAccountDropdown,
  }
  return FBAccountDropdown;
});
