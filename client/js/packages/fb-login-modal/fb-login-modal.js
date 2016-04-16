define('fbloginmodal',
  ['jquery', 'text!./html/fb-login-modal.html'],
  function($, modalHtml) {
  FBLoginModal = function(firebase, callback) {
    var fb = firebase;
    var cb = callback;
    var go = false;
    this.$fbloginmodal = $('.fb-login-modal');

    function firebase_login() {
      go = true;
      var fb_email = $("#fb_email").val();
      var fb_password = $("#fb_password").val();
      fb.authWithPassword({
        email: fb_email, password: fb_password
      }, function(error, authData) {
        if (cb) {
          cb(error, authData);
        }
      });
    }

    function firebase_login_cancel() {
      cb("Firebase login cancelled", null);
    }

    function init() {
      this.$fbloginmodal.html(modalHtml);
      $("#fb_login").click(firebase_login);
    }

    $('.fb-login-modal').addClass('modal fade');
    $('.fb-login-modal').prop('role', 'dialog');
    $('.fb-login-modal').on('hidden.bs.modal', function() {
      if (!go) {
        firebase_login_cancel();
      }
      go = false;
    });
    init.apply(this);
  }

  FBLoginModal.prototype = {
    constructor: FBLoginModal,
  }

  return FBLoginModal;
});
