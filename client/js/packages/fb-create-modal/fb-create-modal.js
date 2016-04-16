define('fbcreatemodal',
  ['jquery', 'text!./html/fb-create-modal.html'],
  function($, modalHtml) {
  FBCreateModal = function(firebase, callback) {
    var fb = firebase;
    var cb = callback;
    var go = false;
    this.$fbcreatemodal = $('.fb-create-modal');

    function validate() {
      $("#fb_create").prop('disabled', !(
        $("#fb_new_email").val() &&
        $("#fb_new_pwd").val() &&
        $("#fb_new_pwd_again").val() &&
        $("#fb_new_pwd").val() === $("#fb_new_pwd_again").val())
      );
    }

    function firebase_create() {
      go = true;
      var fb_reset_email = $("#fb_create_email").val();
      fb.createUser({
        email: fb_new_email,
        password: fb_new_pwd
      }, function(error, authData) {
        if (cb) {
          cb(error, authData);
        }
      });
    }

    function firebase_create_cancel() {
      cb("Firebase reset account cancelled", null);
    }

    function init() {
      this.$fbcreatemodal.html(modalHtml);
      $("#fb_create_email").change(validate);
      $("#fb_create").click(firebase_create);
    }

    $('.fb-create-modal').addClass('modal fade');
    $('.fb-create-modal').prop('role', 'dialog');
    $('.fb-create-modal').on('hidden.bs.modal', function() {
      if (!go) {
        firebase_create_cancel();
      }
      go = false;
    });
    init.apply(this);
  }

  FBCreateModal.prototype = {
    constructor: FBCreateModal,
  }

  return FBCreateModal;
});
