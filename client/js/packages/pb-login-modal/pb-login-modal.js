define('pbloginmodal',
  ['jquery', 'text!pbloginmodal/html/pb-login-modal.html'],
  function($, modalHtml) {
  PBLoginModal = function(particleBaseInstance, callback) {
    var pb = particleBaseInstance;
    var cb = callback;
    var go = false;
    this.$pbloginmodal = $('.pb-login-modal');

    function particle_login() {
      go = true;
      var username = $("#particle_email").val();
      var password = $("#particle_password").val();
      pb.bindAccessToken(username, password, function(status) {
        if (cb) {
          cb(status);
        }
      });
    }

    function particle_login_cancel() {
      cb(ParticleBase.ERROR_CANCEL);
    }

    function init() {
      this.$pbloginmodal.html(modalHtml);
      $("#particle_login").click(particle_login);
    }

    $('.pb-login-modal').addClass('modal fade');
    $('.pb-login-modal').prop('role', 'dialog');
    $('.pb-login-modal').on('hidden.bs.modal', function() {
      if (!go) {
        particle_login_cancel();
      }
      go = false;
    });
    init.apply(this);
  }

  PBLoginModal.prototype = {
    constructor: PBLoginModal,
  }

  return PBLoginModal;
});
