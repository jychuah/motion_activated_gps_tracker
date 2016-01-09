#Firebase Configuration

Firebase is the data and push notification backend for Motorbike Tracker. It powers the Nest thermostat system, it's free for hackers and supports many end user platforms.

### Step 1: Firebase

Get a Firebase account and set up a new Firebase with the Development Only plan at [Firebase.com](http://firebase.com). Write down the full base URL of your firebase. You will need to put it in your front end's <code>js/elements.js</code> file.

### Step 2: Authentication Configuration

In the Logins and Auth configuration, enable E-mail & Password authentication and Anonymous authentication.

### Step 3: Secrets

In the secrets configuration, copy your secret somewhere safe. You will need to put it in your front end's <code>index.php</code> file.

### Step 4: Rules

Overwrite the Security & Rules contents with what's in [rules.json](./rules.json). Don't forget to hit save.
