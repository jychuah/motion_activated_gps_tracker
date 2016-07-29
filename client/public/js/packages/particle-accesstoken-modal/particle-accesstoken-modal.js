define('particleaccesstokenmodal',
  ['jquery',
  'particle',
  'text!./html/particle-accesstoken-modal.html'],
  function($, Particle, modalHtml) {
  ParticleAccessTokenModal = function(callback) {
    var particle = new Particle();
    var cb = callback;
    var go = false;
    this.$particleaccesstokenmodal = $('.particle-accesstoken-modal');

    function particle_login() {
      go = true;
      var email = $('[particle-accesstoken-modal="email"]').val();
      var pwd = $('[particle-accesstoken-modal="password"]').val();

      var xhr = new XMLHttpRequest();
      xhr.open("POST", "https://api.particle.io/oauth/token", true);
      xhr.setRequestHeader("Accept", "*/*");
      xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
      xhr.setRequestHeader ("Authorization", "Basic " + btoa("particle:particle"));
      var ref = this;
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 0 && xhr.status == 0) {
          cb(null);
          return false;
        };
        if (xhr.readyState == 4) {
          var data = xhr.status == 200 ? JSON.parse(xhr.responseText) : null;
          if (data && "access_token" in data) {
            cb(data.access_token);
          } else {
            cb(null);
          }
        }
      };
      xhr.send("grant_type=password&expires_in=0&username=" +
          encodeURIComponent(email) + "&password=" +
          encodeURIComponent(pwd));
    }

    function particle_login_cancel() {
      cb(null);
    }

    function init() {
      this.$particleaccesstokenmodal.html(modalHtml);
      $('[particle-accesstoken-modal="login"]').click(particle_login);
    }

    $('.particle-accesstoken-modal').addClass('modal fade');
    $('.particle-accesstoken-modal').prop('role', 'dialog');
    $('.particle-accesstoken-modal').on('hidden.bs.modal', function() {
      if (!go) {
        particle_login_cancel();
      }
      go = false;
    });
    init.apply(this);
  }

  ParticleAccessTokenModal.prototype = {
    constructor: ParticleAccessTokenModal,
  }

  return ParticleAccessTokenModal;
});
