define('particleloginmodal',
  ['jquery',
  'particle',
  'text!./html/particle-login-modal.html'],
  function($, Particle, modalHtml) {
  ParticleLoginModal = function(callback) {
    var particle = new Particle();
    var cb = callback;
    var go = false;
    this.$particleloginmodal = $('.particle-login-modal');

    function particle_login() {
      go = true;
      var email = $('[particle-login-modal="email"]').val();
      var pwd = $('[particle-login-modal="password"]').val();
      particle.login({ username: email, password: pwd }).then(
        function(data) {
          cb(data.body.access_token);
        },
        function(err) {
          cb(null);
        }
      );
    }

    function particle_login_cancel() {
      cb(null);
    }

    function init() {
      this.$particleloginmodal.html(modalHtml);
      $('[particle-login-modal="login"]').click(particle_login);
    }

    $('.particle-login-modal').addClass('modal fade');
    $('.particle-login-modal').prop('role', 'dialog');
    $('.particle-login-modal').on('hidden.bs.modal', function() {
      if (!go) {
        particle_login_cancel();
      }
      go = false;
    });
    init.apply(this);
  }

  ParticleLoginModal.prototype = {
    constructor: ParticleLoginModal,
  }

  return ParticleLoginModal;
});
