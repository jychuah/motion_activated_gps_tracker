define('fbresetmodal',
  ['jquery', 'text!fbresetmodal/html/fb-reset-modal.html'],
  function($, modalHtml) {
  FBResetModal = function(firebase, callback) {
    var fb = firebase;
    var cb = callback;
    var go = false;
    this.$fbresetmodal = $('.fb-reset-modal');

    function validate() {
      $("#fb_reset").prop('disabled', !(
        $("#fb_reset_email").val())
      );
    }

    function firebase_reset() {
      go = true;
      var fb_reset_email = $("#fb_reset_email").val();
      fb.resetPassword({
        email: fb_reset_email
      }, function(error) {
        if (cb) {
          cb(error);
        }
      });
    }

    function firebase_reset_cancel() {
      cb("Firebase reset account cancelled");
    }

    function init() {
      this.$fbresetmodal.html(modalHtml);
      $("#fb_reset_email").change(validate);
      $("#fb_reset").click(firebase_reset);
    }

    $('.fb-reset-modal').addClass('modal fade');
    $('.fb-reset-modal').prop('role', 'dialog');
    $('.fb-reset-modal').on('hidden.bs.modal', function() {
      if (!go) {
        firebase_reset_cancel();
      }
      go = false;
    });
    init.apply(this);
  }

  FBResetModal.prototype = {
    constructor: FBResetModal,
  }

  return FBResetModal;
});
