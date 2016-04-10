define('pbloginmodal',
  ['jquery', 'text!pbloginmodal/html/pb-login-modal.html'],
  function($, modalHtml, itemHtml) {
  PBLoginModal = function(particleBaseInstance, callback) {
    var pb = particleBaseInstance;
    var cb = callback;
    this.$pbloginmodal = $('.pb-login-modal');

    function particle_login() {
      var username = $("#particle_email").val();
      var password = $("#particle_password").val();
      pb.bindAccessToken(username, password, function(status) {
        if (cb) {
          cb(status);
        }
      });
    }

    function particle_login_cancel() {
      callback(ParticleBase.ERROR_CANCEL);
    }

    function init() {
      this.$pbloginmodal.html(modalHtml);
      $("#particle_login").click(particle_login);
      $("#particle_login_cancel").click(particle_login_cancel);
    }

    $('.pb-login-modal').addClass('modal fade');
    $('.pb-login-modal').prop('role', 'dialog');
    init.apply(this);
  }

  PBLoginModal.prototype = {
    constructor: PBLoginModal,
  }

  return PBLoginModal;
});
