define('fbchangepwmodal',
  ['jquery', 'text!fbchangepwmodal/html/fb-changepw-modal.html'],
  function($, modalHtml) {
  FBChangePwModal = function(firebase, callback) {
    var fb = firebase;
    var cb = callback;
    var go = false;
    this.$fbchangepwmodal = $('.fb-changepw-modal');

    function validate() {
      $("#fb_changepw").prop('disabled', !(
        $("#fb_currentpwd").val() &&
        $("#fb_setpwd").val() &&
        $("#fb_setpwdagain").val() &&
        $("#fb_setpwd").val() === $("#fb_setpwdagain").val())
      );
    }

    function firebase_changepw() {
      go = true;
      var fbemail = fb.getAuth().password.email;
      var oldpw = $("#fb_currentpwd").val();
      var newpw = $("#fb_setpwd").val();
      var newpwagain = $("#fb_setpwdagain").val();
      fb.changePassword({
        email: fbemail,
        oldPassword: oldpw,
        newPassword: newpw
      }, function(error) {
        if (cb) {
          cb(error);
        }
      });
    }

    function firebase_changepw_cancel() {
      cb("Firebase change password cancelled");
    }

    function init() {
      this.$fbchangepwmodal.html(modalHtml);
      $("#fb_currentpwd").change(validate);
      $("#fb_setpwd").change(validate);
      $("#fb_setpwdagain").change(validate);
      $("#fb_changepw").click(firebase_changepw);
    }

    $('.fb-changepw-modal').addClass('modal fade');
    $('.fb-changepw-modal').prop('role', 'dialog');
    $('.fb-changepw-modal').on('hidden.bs.modal', function() {
      if (!go) {
        firebase_changepw_cancel();
      }
      go = false;
    });
    init.apply(this);
  }

  FBChangePwModal.prototype = {
    constructor: FBChangePwModal,
  }

  return FBChangePwModal;
});
