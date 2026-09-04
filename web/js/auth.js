// Auth Firebase (frontend)
const Auth = (() => {
  const API_KEY = (window.FIREBASE_CONFIG && window.FIREBASE_CONFIG.apiKey) || "AIzaSyDkFL-rIPJP9gK9uUnSenAyYT-ZWtMRrds";
  const SIGNUP_URL = `https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=${API_KEY}`;
  const LOGIN_URL = `https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=${API_KEY}`;
  const REFRESH_URL = `https://securetoken.googleapis.com/v1/token?key=${API_KEY}`;

  let listeners = [];
  let currentUser = null;

  function notify(user) {
    currentUser = user;
    listeners.forEach(fn => fn(user));
  }

  async function request(url, body) {
    const res = await fetch(url, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body)
    });
    const data = await res.json();
    if (!res.ok) {
      const msg = data.error?.message || data.error?.errors?.[0]?.message || JSON.stringify(data.error) || "Error de auth";
      throw new Error(msg);
    }
    return data;
  }

  async function signup(email, password) {
    const data = await request(SIGNUP_URL, { email, password, returnSecureToken: true });
    return setSession(data);
  }

  async function login(email, password) {
    const data = await request(LOGIN_URL, { email, password, returnSecureToken: true });
    return setSession(data);
  }

  function setSession(data) {
    const user = {
      uid: data.localId,
      email: data.email,
      idToken: data.idToken,
      refreshToken: data.refreshToken,
      expiresAt: Date.now() + data.expiresIn * 1000
    };
    localStorage.setItem("fb_user", JSON.stringify(user));
    notify(user);
    return user;
  }

  async function refreshToken() {
    const stored = localStorage.getItem("fb_user");
    if (!stored) return null;
    const user = JSON.parse(stored);
    try {
      const data = await request(REFRESH_URL, {
        grant_type: "refresh_token",
        refresh_token: user.refreshToken
      });
      user.idToken = data.access_token;
      user.expiresAt = Date.now() + data.expires_in * 1000;
      localStorage.setItem("fb_user", JSON.stringify(user));
      notify(user);
      return user;
    } catch {
      logout();
      return null;
    }
  }

  async function getValidToken() {
    const stored = localStorage.getItem("fb_user");
    if (!stored) return null;
    const user = JSON.parse(stored);
    if (Date.now() > user.expiresAt - 60000) return refreshToken().then(u => u?.idToken);
    return user.idToken;
  }

  function logout() {
    localStorage.removeItem("fb_user");
    notify(null);
  }

  function getCurrentUser() {
    const stored = localStorage.getItem("fb_user");
    return stored ? JSON.parse(stored) : null;
  }

  function onAuthChange(fn) {
    listeners.push(fn);
    const u = getCurrentUser();
    if (u) fn(u);
    return () => { listeners = listeners.filter(l => l !== fn); };
  }

  async function ensureToken() {
    const token = await getValidToken();
    if (!token) throw new Error("No autenticado");
    return token;
  }

  return { signup, login, logout, getValidToken, ensureToken, getCurrentUser, onAuthChange };
})();