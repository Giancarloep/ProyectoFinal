"use strict";

const $ = (id) => document.getElementById(id);

function onReady(fn) {
  if (document.readyState === "loading") document.addEventListener("DOMContentLoaded", fn);
  else fn();
}

async function api(ruta, opciones = {}) {
  const token = await Auth.getValidToken();
  const headers = { "Content-Type": "application/json" };
  if (token) headers["Authorization"] = `Bearer ${token}`;
  const resp = await fetch(ruta, {
    headers,
    ...opciones,
  });
  const datos = await resp.json().catch(() => ({}));
  if (!resp.ok) throw new Error(datos.error || "Error del servidor");
  return datos;
}

let toastTimeout = null;
function toast(mensaje, tipo = "") {
  const el = $("toast");
  el.textContent = mensaje;
  el.className = `toast visible ${tipo}`;
  clearTimeout(toastTimeout);
  toastTimeout = setTimeout(() => el.classList.remove("visible"), 2800);
}

function formatoReloj(segundos) {
  const s = Math.max(0, segundos | 0);
  const h = Math.floor(s / 3600);
  const m = Math.floor((s % 3600) / 60);
  const r = s % 60;
  const dos = (n) => String(n).padStart(2, "0");
  return h > 0 ? `${h}:${dos(m)}:${dos(r)}` : `${dos(m)}:${dos(r)}`;
}

function tipoSplit(dias) {
  if (dias <= 3) return "Cuerpo completo";
  if (dias <= 5) return "Torso/Pierna";
  return "Empuje/Traccion/Pierna";
}

function cambiarTab(nombre) {
  document.querySelectorAll(".tab").forEach((t) =>
    t.classList.toggle("activo", t.dataset.tab === nombre)
  );
  document.querySelectorAll(".panel").forEach((p) =>
    p.classList.toggle("activo", p.id === `tab-${nombre}`)
  );
  $("modalSwap").hidden = true;
  $("modalGuia").hidden = true;
}

function initTheme() {
  const btn = $("btnTheme");
  if (!btn) return;
  const saved = localStorage.getItem("theme");
  if (saved === "light") document.documentElement.classList.add("light");
  btn.addEventListener("click", () => {
    const isLight = document.documentElement.classList.toggle("light");
    localStorage.setItem("theme", isLight ? "light" : "dark");
  });
}

onReady(() => {
  initTheme();
  $("tabs").addEventListener("click", (e) => {
    const boton = e.target.closest(".tab");
    if (boton) cambiarTab(boton.dataset.tab);
  });

  // Auth state listener
  Auth.onAuthChange(async (user) => {
    const authScreen = $("authScreen");
    const appContainer = $("appContainer");
    const userEmail = $("userEmail");
    const btnLogout = $("btnLogout");

    if (user) {
      authScreen.hidden = true;
      appContainer.hidden = false;
      userEmail.textContent = user.email;
      userEmail.hidden = false;
      btnLogout.hidden = false;
      await inicializarApp();
    } else {
      authScreen.hidden = false;
      appContainer.hidden = true;
      userEmail.hidden = true;
      btnLogout.hidden = true;
    }
  });

  // Auth form handler
  $("authForm").addEventListener("submit", async (e) => {
    e.preventDefault();
    const email = $("authEmail").value.trim();
    const pass = $("authPass").value;
    const errorEl = $("authError");
    const submitBtn = $("authSubmit");
    const isSignup = submitBtn.dataset.mode === "signup";

    try {
      submitBtn.disabled = true;
      submitBtn.textContent = isSignup ? "Creando cuenta..." : "Entrando...";
      errorEl.hidden = true;
      if (isSignup) await Auth.signup(email, pass);
      else await Auth.login(email, pass);
    } catch (err) {
      errorEl.textContent = err.message;
      errorEl.hidden = false;
    } finally {
      submitBtn.disabled = false;
      submitBtn.textContent = isSignup ? "Registrarse" : "Entrar";
    }
  });

  // Toggle login/signup
  $("authToggle").addEventListener("click", () => {
    const isSignup = $("authSubmit").dataset.mode === "signup";
    const title = $("authTitle");
    const subtitle = $("authSubtitle");
    const submitBtn = $("authSubmit");
    const switchText = $("authSwitchText");
    const toggleBtn = $("authToggle");

    if (isSignup) {
      title.textContent = "Iniciar sesión";
      subtitle.textContent = "Accede para usar la app";
      submitBtn.textContent = "Entrar";
      submitBtn.dataset.mode = "login";
      switchText.textContent = "¿No tienes cuenta?";
      toggleBtn.textContent = "Regístrate";
    } else {
      title.textContent = "Crear cuenta";
      subtitle.textContent = "Regístrate para acceder";
      submitBtn.textContent = "Registrarse";
      submitBtn.dataset.mode = "signup";
      switchText.textContent = "¿Ya tienes cuenta?";
      toggleBtn.textContent = "Inicia sesión";
    }
    $("authError").hidden = true;
  });

  // Logout
  $("btnLogout").addEventListener("click", () => {
    Auth.logout();
  });
});

const inpDias = $("inpDias");
inpDias.addEventListener("input", () => {
  $("outDias").textContent = inpDias.value;
});

let equipoSeleccionado = "gimnasio";

async function cargarEquipos() {
  try {
    const datos = await api("/api/equipos");
    $("listaEquipos").innerHTML = datos.equipos
      .map(
        (e) => `<button type="button" class="chip" data-equipo="${e.clave}">${e.etiqueta}</button>`
      )
      .join("");
    $("listaEquipos").addEventListener("click", (e) => {
      const chip = e.target.closest(".chip");
      if (!chip) return;
      equipoSeleccionado = chip.dataset.equipo;
      marcarEquipo(equipoSeleccionado);
    });
    marcarEquipo(equipoSeleccionado);
  } catch {
    /* servidor caído */
  }
}

function marcarEquipo(clave) {
  document.querySelectorAll("#listaEquipos .chip").forEach((c) =>
    c.classList.toggle("activo", c.dataset.equipo === clave)
  );
}

function construirChips(nombresMusculos) {
  $("chipsMusculos").innerHTML = nombresMusculos
    .map((m) => `<button type="button" class="chip" data-musculo="${m}">${m}</button>`)
    .join("");
}

$("chipsMusculos").addEventListener("click", (e) => {
  const chip = e.target.closest(".chip");
  if (chip) chip.classList.toggle("activo");
});

function chipsSeleccionados() {
  return [...document.querySelectorAll("#chipsMusculos .chip.activo")]
    .map((c) => c.dataset.musculo);
}

function marcarChips(nombres) {
  document.querySelectorAll("#chipsMusculos .chip").forEach((c) => {
    c.classList.toggle("activo", nombres.includes(c.dataset.musculo));
  });
}

async function cargarTabla() {
  try {
    const datos = await api("/api/tabla");
    construirChips(datos.tabla.map((f) => f.musculo));

    const filas = [
      ["MV", "mv"],
      ["MEV", "mev"],
      ["MAV", "mav"],
      ["MRV", "mrv"],
      ["Frecuencia", "frecuencia"],
      ["Reps", "repeticiones"],
      ["RIR", "rir"],
    ];

    const cabecera =
      `<tr><th class="fila-label"></th>` +
      datos.tabla.map((f) => `<th>${f.referencia}</th>`).join("") +
      `</tr>`;

    const cuerpo = filas
      .map(
        ([etiqueta, clave]) =>
          `<tr><td class="fila-label">${etiqueta}</td>` +
          datos.tabla.map((f) => `<td>${f[clave]}</td>`).join("") +
          `</tr>`
      )
      .join("");

    $("contenedorTabla").innerHTML =
      `<table class="tabla-hyper">${cabecera}${cuerpo}</table>`;
  } catch {
    /* servidor caído */
  }
}

let objetivoSeleccionado = "progresar";
let objetivosCache = [];

function evaluarNotaDias() {
  const nota = $("notaDias");
  const info = objetivosCache.find((o) => o.clave === objetivoSeleccionado);
  if (!info) return;
  const dias = Number(inpDias.value);
  const dentro = dias >= info.diasRecomendados[0] && dias <= info.diasRecomendados[1];
  nota.hidden = false;
  nota.classList.toggle("alerta", !dentro);
  nota.textContent = dentro
    ? `Recomendado para "${info.etiqueta}": ${info.diasRecomendados[0]}-${info.diasRecomendados[1]} días/semana. Estás en rango.`
    : `Atención: para "${info.etiqueta}" se recomiendan ${info.diasRecomendados[0]}-${info.diasRecomendados[1]} días/semana y elegiste ${dias}.`;
}

function seleccionarObjetivo(clave, ajustarSlider) {
  objetivoSeleccionado = clave;
  document.querySelectorAll(".objetivo").forEach((c) =>
    c.classList.toggle("activo", c.dataset.clave === clave)
  );
  const info = objetivosCache.find((o) => o.clave === clave);
  if (info && ajustarSlider) {
    inpDias.value = Math.round((info.diasRecomendados[0] + info.diasRecomendados[1]) / 2);
    $("outDias").textContent = inpDias.value;
  }
  evaluarNotaDias();
}

async function cargarObjetivos() {
  try {
    const datos = await api("/api/objetivos");
    objetivosCache = datos.objetivos;
    $("listaObjetivos").innerHTML = datos.objetivos
      .map(
        (o) => `<button type="button" class="objetivo" data-clave="${o.clave}">
          <span class="obj-zona ${o.zona}">${o.zona}</span>
          <span class="obj-nombre">${o.etiqueta}</span>
          <span class="obj-desc">${o.descripcion}</span>
          <span class="obj-dias">Recomendado: ${o.diasRecomendados[0]}-${o.diasRecomendados[1]} días/semana</span>
        </button>`
      )
      .join("");
    $("listaObjetivos").addEventListener("click", (e) => {
      const tarjeta = e.target.closest(".objetivo");
      if (tarjeta) seleccionarObjetivo(tarjeta.dataset.clave, true);
    });
    if (!document.querySelector(".objetivo.activo")) {
      seleccionarObjetivo(objetivoSeleccionado, true);
    }
  } catch {
    /* servidor caído */
  }
}

inpDias.addEventListener("change", () => {
  if ($("tipoSplit").hidden === false) evaluarNotaDias();
});

$("btnGuardarPerfil").addEventListener("click", async () => {
  const nombre = $("inpNombre").value.trim();
  if (!nombre) return toast("Escribe tu nombre primero", "error");
  try {
    const perfil = await api("/api/perfil", {
      method: "POST",
      body: JSON.stringify({
        nombre,
        diasDisponibles: Number(inpDias.value),
        pesoCorporal: Number($("inpPesoCorporal").value) || 0,
        musculosPrioritarios: chipsSeleccionados(),
        objetivo: objetivoSeleccionado,
        equipo: equipoSeleccionado,
      }),
    });
    mostrarPerfil(perfil);
    cargarTiers();
    toast(`Perfil guardado. Split: ${perfil.tipoSplit}`, "ok");
  } catch (err) {
    toast(err.message, "error");
  }
});

function mostrarPerfil(perfil) {
  $("inpNombre").value = perfil.nombre;
  inpDias.value = perfil.diasDisponibles;
  $("outDias").textContent = perfil.diasDisponibles;
  $("inpPesoCorporal").value = perfil.pesoCorporal || "";
  marcarChips(perfil.musculosPrioritarios || []);
  if (perfil.objetivo && perfil.objetivo.clave) {
    seleccionarObjetivo(perfil.objetivo.clave, false);
  }
  if (perfil.equipo && perfil.equipo.clave) {
    equipoSeleccionado = perfil.equipo.clave;
    marcarEquipo(equipoSeleccionado);
  }
  const insignia = $("tipoSplit");
  const prioridad = (perfil.musculosPrioritarios || []).length > 0
    ? ` · prioridad: ${perfil.musculosPrioritarios.join(", ")}`
    : "";
  insignia.textContent = `Split: ${perfil.tipoSplit} (${perfil.diasDisponibles} días/semana)${prioridad}`;
  insignia.hidden = false;
}

async function cargarPerfil() {
  try {
    mostrarPerfil(await api("/api/perfil"));
  } catch {
    /* primera vez sin perfil */
  }
}

function ejercicioFila(ej) {
  return `<li>
    <span class="ej-nombre">${ej.nombre}${ej.rir ? ` <small class="ej-rir">RIR ${ej.rir}</small>` : ""}</span>
    <span class="ej-grupo">${ej.grupoMuscular}</span>
    <span class="ej-volumen">${ej.series}×${ej.repeticiones}</span>
  </li>`;
}

function pintarRutina(rutina) {
  $("subtituloRutina").textContent =
    `${rutina.usuario} · ${rutina.dias.length} día(s)/semana`;
  $("contenedorRutina").innerHTML = rutina.dias
    .map(
      (dia, i) => `<article class="dia">
        <div class="dia-cabecera">${dia.nombre}</div>
        <ul>${dia.ejercicios
          .map(
            (ej, j) => `<li>
              <span class="ej-nombre">${ej.nombre}${ej.rir ? ` <small class="ej-rir">RIR ${ej.rir}</small>` : ""}</span>
              <span class="ej-grupo">${ej.grupoMuscular}</span>
              <span class="ej-volumen">${ej.series}×${ej.repeticiones}</span>
              <span class="ej-acciones">
                <button class="guia" data-ejercicio="${ej.nombre}" title="Ver cómo se hace">¿cómo?</button>
                <button class="swap" data-dia="${i}" data-indice="${j}">cambiar</button>
              </span>
            </li>`
          )
          .join("")}</ul>
      </article>`
    )
    .join("");
  $("msgRutina").hidden = true;

  const nombres = new Set();
  rutina.dias.forEach((d) => d.ejercicios.forEach((e) => nombres.add(e.nombre)));
  $("listaEjercicios").innerHTML = [...nombres]
    .map((n) => `<option value="${n}">`)
    .join("");
}

$("contenedorRutina").addEventListener("click", (e) => {
  const boton = e.target.closest(".swap");
  if (!boton) return;
  abrirModalSwap(Number(boton.dataset.dia), Number(boton.dataset.indice));
});

$("contenedorRutina").addEventListener("click", (e) => {
  const boton = e.target.closest(".guia");
  if (!boton) return;
  abrirModalGuia(boton.dataset.ejercicio);
});

let guiaDatos = null;
let guiaPromesa = null;

function cargarGuia() {
  if (guiaDatos) return Promise.resolve(guiaDatos);
  if (!guiaPromesa) {
    guiaPromesa = fetch("/data/guia-ejercicios.json")
      .then((r) => r.json())
      .then((datos) => {
        guiaDatos = datos.ejercicios;
        return guiaDatos;
      })
      .catch(() => {
        guiaPromesa = null;
        return null;
      });
  }
  return guiaPromesa;
}

async function abrirModalGuia(nombre) {
  const datos = await cargarGuia();
  if (!datos) return toast("No se pudo cargar la guía de ejercicios", "error");
  const guia = datos[nombre];
  if (!guia) return toast(`Sin guía para "${nombre}"`, "error");

  $("guiaTitulo").textContent = nombre;
  $("guiaMeta").textContent = `${guia.musculo} · ${guia.equipo}`;
  $("guiaImagenes").innerHTML = (guia.imagenes || [])
    .map(
      (src) =>
        `<img src="${src}" alt="${nombre}" loading="lazy" referrerpolicy="no-referrer">`
    )
    .join("");
  $("guiaSinFoto").hidden = (guia.imagenes || []).length > 0;
  $("guiaPasos").innerHTML = (guia.pasos || [])
    .map((paso) => `<li>${paso}</li>`)
    .join("");
  $("modalGuia").hidden = false;
}

$("btnCerrarGuia").addEventListener("click", () => {
  $("modalGuia").hidden = true;
});
$("modalGuia").addEventListener("click", (e) => {
  if (e.target === $("modalGuia")) $("modalGuia").hidden = true;
});

async function abrirModalSwap(dia, indice) {
  try {
    const datos = await api(
      `/api/rutina/alternativas?dia=${dia}&indice=${indice}`
    );
    if (!datos.alternativas.length) {
      return toast("No hay alternativas para ese ejercicio con tu equipo", "error");
    }
    const rutina = await api("/api/rutina");
    const actual = rutina.dias[dia].ejercicios[indice];
    $("modalActual").textContent =
      `Ejercicio actual: ${actual.nombre} (${actual.grupoMuscular})`;
    $("modalOpciones").innerHTML = datos.alternativas
      .map(
        (alt, k) => `<button type="button" class="opcion-swap" data-dia="${dia}" data-indice="${indice}" data-nuevo="${alt.nombre}">
          <span>${alt.nombre}</span>
          <span class="num">${alt.series}×${alt.repeticiones}</span>
        </button>`
      )
      .join("");
    $("modalSwap").hidden = false;
  } catch (err) {
    toast(err.message, "error");
  }
}

$("modalOpciones")?.addEventListener?.("click", async (e) => {
  const opcion = e.target.closest(".opcion-swap");
  if (!opcion) return;
  try {
    const rutina = await api("/api/rutina/cambiar", {
      method: "POST",
      body: JSON.stringify({
        dia: Number(opcion.dataset.dia),
        indice: Number(opcion.dataset.indice),
        nuevo: opcion.dataset.nuevo,
      }),
    });
    pintarRutina(rutina);
    cargarVolumen();
    $("modalSwap").hidden = true;
    toast(`Ejercicio cambiado a: ${opcion.dataset.nuevo}`, "ok");
  } catch (err) {
    toast(err.message, "error");
  }
});

$("btnCerrarModal").addEventListener("click", () => {
  $("modalSwap").hidden = true;
});
$("modalSwap").addEventListener("click", (e) => {
  if (e.target === $("modalSwap")) $("modalSwap").hidden = true;
});

$("btnGenerarRutina").addEventListener("click", async () => {
  try {
    const rutina = await api("/api/rutina/generar", { method: "POST" });
    pintarRutina(rutina);
    cambiarTab("rutina");
    cargarVolumen();
    toast("¡Rutina generada con la tabla de hipertrofia!", "ok");
  } catch (err) {
    toast(err.message, "error");
  }
});

$("btnDeshacer").addEventListener("click", async () => {
  try {
    const rutina = await api("/api/rutina/deshacer", { method: "POST" });
    pintarRutina(rutina);
    cargarVolumen();
    toast("Cambio deshecho (pila de historial)", "ok");
  } catch (err) {
    toast(err.message, "error");
  }
});

async function cargarVolumen() {
  try {
    const datos = await api("/api/volumen");
    $("msgVolumen").hidden = true;
    $("subtituloVolumen").textContent =
      "Series por semana según tu rutina actual, evaluadas contra la tabla.";

    $("contenedorVolumen").innerHTML = datos.volumen
      .map((v) => {
        const ancho = Math.min(100, Math.round((v.seriesSemana / 26) * 100));
        return `<div class="vol-card">
          <div class="vol-cabecera">
            <span>
              <div class="vol-musculo">${v.musculo}</div>
              <div class="vol-ref">${v.referencia}</div>
            </span>
            <span class="estado ${v.estado}">${v.estado}</span>
          </div>
          <div class="vol-series">${v.seriesSemana} <small>series/semana</small></div>
          <div class="vol-barra"><i class="barra-${v.estado}" style="width:${ancho}%"></i></div>
          <div class="vol-rango">MEV ${v.mev} · MAV ${v.mav}</div>
        </div>`;
      })
      .join("");
  } catch {
    $("msgVolumen").hidden = false;
    $("contenedorVolumen").innerHTML = "";
  }
}

async function cargarRutina() {
  try {
    pintarRutina(await api("/api/rutina"));
  } catch {
    /* sin rutina todavía */
  }
}

$("btnRegistrarPr").addEventListener("click", async () => {
  const ejercicio = $("inpEjercicio").value.trim();
  const pesoLb = Number($("inpPeso").value);
  const repeticiones = Number($("inpReps").value);
  if (!ejercicio) return toast("Indica el ejercicio", "error");
  if (!(pesoLb > 0)) return toast("Indica un peso válido", "error");
  if (!(repeticiones >= 1)) return toast("Indica las repeticiones", "error");

  try {
    const resultado = await api("/api/prs", {
      method: "POST",
      body: JSON.stringify({ ejercicio, pesoKg: pesoLb, repeticiones }),
    });
    if (resultado.tierSubio) {
      pitido();
      toast(`¡TRANSFORMACION! Subiste a ${resultado.tier} (${resultado.ratio.toFixed(2)}× tu peso)`, "pr");
    } else if (resultado.nuevoPR) {
      toast(`NUEVO PR · ${ejercicio} ${pesoLb}lb × ${repeticiones} (1RM est. ${resultado.rm1Estimado.toFixed(1)} lb)`, "pr");
      pitido();
    } else {
      toast(`Registrado: ${ejercicio} ${pesoLb}lb × ${repeticiones}`, "ok");
    }
    $("inpPeso").value = "";
    $("inpReps").value = "";
    await cargarPRs();
    cargarTiers();
  } catch (err) {
    toast(err.message, "error");
  }
});

let tiersCache = [];

async function cargarTiers() {
  const tarjeta = $("tarjetaTiers");
  let datos;
  try {
    datos = await api("/api/tiers");
  } catch {
    tarjeta.hidden = true;
    return;
  }
  tiersCache = datos.tiers || [];

  if (!datos.conPeso) {
    tarjeta.hidden = true;
    return;
  }

  tarjeta.hidden = false;

const insignia = $("tierInsignia");
  insignia.textContent = datos.tierActual || "Saiyajin base";
  insignia.className = `tier-insignia tier-${datos.colorActual || "base"}`;
  $("tierSubtitulo").textContent =
    `Mejor ratio: ${datos.mejorRatio.toFixed(2)}× tu peso (${datos.mejorEjercicio}) · peso corporal ${datos.pesoCorporal} lb`;

  const barra = $("tierBarra");
  const nota = $("tierSiguiente");
  if (datos.siguienteUmbral > 0) {
    const avance = Math.min(100, Math.round((datos.mejorRatio / datos.siguienteUmbral) * 100));
    barra.style.width = `${avance}%`;
    barra.className = `barra-${datos.colorActual || "base"}`;
    const falta = (datos.siguienteUmbral - datos.mejorRatio) * datos.pesoCorporal;
    nota.textContent =
      `Te faltan ${falta.toFixed(1)} lb de 1RM para alcanzar ${datos.siguienteTier} (${datos.siguienteUmbral}× tu peso).`;
  } else {
    barra.style.width = "100%";
    barra.className = "barra-supersaiyajin";
    nota.textContent = "¡Has alcanzado el tier máximo! ¡Modo Super Saiyajin desbloqueado!";
  }
}

async function cargarPRs() {
  let tiersPorEjercicio = {};
  try {
    const datos = await api("/api/tiers");
    if (datos.conPeso) {
      (datos.porEjercicio || []).forEach((t) => {
        tiersPorEjercicio[t.ejercicio] = t;
      });
    }
  } catch {
    /* sin tiers */
  }

  try {
    const datos = await api("/api/prs");
    const cuerpo = $("tablaPrs").querySelector("tbody");
    cuerpo.innerHTML = datos.prs
      .map(
        (pr) => {
          const t = tiersPorEjercicio[pr.ejercicio];
          const tier = t
            ? `<span class="tier-chip tier-${t.color}">${t.tier}</span>`
            : "<span class='tier-chip tier-sinpeso'>—</span>";
          return `<tr>
          <td>${pr.ejercicio}</td>
          <td class="num">${pr.pesoKg} lb × ${pr.repeticiones}</td>
          <td class="num">${pr.rm1Estimado.toFixed(1)}</td>
          <td>${tier}</td>
          <td>${pr.fecha}</td>
        </tr>`;
        }
      )
      .join("");
    $("msgPrs").hidden = datos.prs.length > 0;
  } catch {
    /* sin conexión */
  }
}

let tTotal = 60;
let tRestante = 60;
let tId = null;

const CIRCUNFERENCIA = 2 * Math.PI * 95;

function pintarTimer() {
  $("textoTimer").textContent = formatoReloj(tRestante);
  const progreso = tTotal > 0 ? tRestante / tTotal : 0;
  const anillo = $("anilloProgreso");
  anillo.style.strokeDasharray = CIRCUNFERENCIA;
  anillo.style.strokeDashoffset = String(CIRCUNFERENCIA * (1 - progreso));
  anillo.classList.toggle("terminado", tRestante === 0 && tTotal > 0);
}

function detenerTimer() {
  clearInterval(tId);
  tId = null;
  $("btnTimerToggle").textContent = "Iniciar";
}

$("btnTimerToggle").addEventListener("click", () => {
  if (tId) {
    detenerTimer();
    $("btnTimerToggle").textContent = "Continuar";
    return;
  }
  if (tRestante === 0) tRestante = tTotal;
  $("btnTimerToggle").textContent = "Pausar";
  tId = setInterval(() => {
    tRestante--;
    pintarTimer();
    if (tRestante <= 0) {
      detenerTimer();
      tRestante = 0;
      pitido();
      toast("Descanso terminado. ¡A la siguiente serie!", "pr");
    }
  }, 1000);
});

$("btnTimerReset").addEventListener("click", () => {
  detenerTimer();
  tRestante = tTotal;
  pintarTimer();
});

document.querySelectorAll(".preset").forEach((boton) => {
  boton.addEventListener("click", () => {
    document.querySelectorAll(".preset").forEach((b) => b.classList.remove("activo"));
    boton.classList.add("activo");
    detenerTimer();
    tTotal = Number(boton.dataset.segundos);
    tRestante = tTotal;
    pintarTimer();
  });
});

function pitido() {
  try {
    const ctx = new (window.AudioContext || window.webkitAudioContext)();
    [0, 0.28, 0.56].forEach((inicio) => {
      const osc = ctx.createOscillator();
      const ganancia = ctx.createGain();
      osc.connect(ganancia);
      ganancia.connect(ctx.destination);
      osc.type = "sine";
      osc.frequency.value = 880;
      ganancia.gain.setValueAtTime(0.22, ctx.currentTime + inicio);
      osc.start(ctx.currentTime + inicio);
      osc.stop(ctx.currentTime + inicio + 0.18);
    });
  } catch {
    /* navegador sin audio */
  }
}

let sesionIntervalo = null;

$("btnSesion").addEventListener("click", async () => {
  if (sesionIntervalo) return cerrarSesion();
  try {
    await api("/api/sesion/iniciar", { method: "POST" });
    $("relojSesion").hidden = false;
    $("relojSesion").textContent = "00:00";
    $("btnSesion").textContent = "Terminar";
    sesionIntervalo = setInterval(async () => {
      try {
        const estado = await api("/api/sesion");
        $("relojSesion").textContent = formatoReloj(estado.segundos);
      } catch {
        /* servidor caído */
      }
    }, 1000);
    toast("Sesión de entrenamiento iniciada", "ok");
  } catch (err) {
    toast(err.message, "error");
  }
});

async function cerrarSesion() {
  clearInterval(sesionIntervalo);
  sesionIntervalo = null;
  let total = 0;
  try {
    const fin = await api("/api/sesion/terminar", { method: "POST" });
    total = fin.segundos;
  } catch {
    /* nada */
  }
  const minutos = Math.round(total / 60);
  toast(`Sesión terminada: ${formatoReloj(total)} (${minutos} min)`, "ok");
  $("relojSesion").hidden = true;
  $("btnSesion").textContent = "Iniciar entreno";
  cargarSesiones();
}

async function cargarSesiones() {
  try {
    const datos = await api("/api/sesiones");
    const lista = $("listaSesiones");
    lista.innerHTML = datos.sesiones
      .map(
        (s) => `<li><span>${s.fecha}</span><span class="num">${formatoReloj(s.segundos)}</span></li>`
      )
      .join("");
    $("msgSesiones").hidden = datos.sesiones.length > 0;
  } catch {
    /* sin conexión */
  }
}

async function refrescarEstadoSesion() {
  try {
    const estado = await api("/api/sesion");
    if (!estado.activa) return;
    $("relojSesion").hidden = false;
    $("relojSesion").textContent = formatoReloj(estado.segundos);
    $("btnSesion").textContent = "Terminar";
    sesionIntervalo = setInterval(async () => {
      try {
        const s = await api("/api/sesion");
        $("relojSesion").textContent = formatoReloj(s.segundos);
      } catch {
        /* servidor caído */
      }
    }, 1000);
  } catch {
    /* sin conexión */
  }
}

function agregarMensaje(tipo, texto) {
  const div = document.createElement("div");
  div.className = `msg ${tipo}`;
  div.textContent = texto;
  $("chatMensajes").appendChild(div);
  $("chatMensajes").scrollTop = $("chatMensajes").scrollHeight;
  return div;
}

$("formChat").addEventListener("submit", async (e) => {
  e.preventDefault();
  const texto = $("inpChat").value.trim();
  if (!texto) return;
  agregarMensaje("user", texto);
  $("inpChat").value = "";
  const burbuja = agregarMensaje("bot", "Escribiendo...");
  try {
    const datos = await api("/api/asistente", {
      method: "POST",
      body: JSON.stringify({ pregunta: texto }),
    });
    burbuja.textContent = datos.respuesta;
    $("chatMensajes").scrollTop = $("chatMensajes").scrollHeight;
  } catch (err) {
    burbuja.textContent = `Error: ${err.message}`;
  }
});

document.querySelectorAll(".sugerencias button").forEach((b) =>
  b.addEventListener("click", () => {
    $("inpChat").value = b.dataset.pregunta;
    $("formChat").requestSubmit();
  })
);

async function cargarEstadoIa() {
  const el = $("estadoIa");
  try {
    const e = await api("/api/asistente/estado");
    if (e.modeloLocal) {
      el.textContent = `IA generativa activa (${e.modelo}) · respaldo experto local`;
      el.classList.add("activa");
    } else {
      el.textContent =
        "Modo experto local · instala Ollama + modelo \"llama3.2\" para IA generativa";
    }
  } catch {
    el.textContent = "Modo experto local";
  }
}

$("modalSwap").hidden = true;
$("modalGuia").hidden = true;
pintarTimer();

async function inicializarApp() {
  await Promise.all([
    cargarTabla(),
    cargarObjetivos(),
    cargarEquipos()
  ]);
  await cargarPerfil();
  await cargarRutina();
  await cargarPRs();
  cargarTiers();
  await cargarVolumen();
  cargarEstadoIa();
  refrescarEstadoSesion();
  cargarSesiones();
  agregarMensaje(
    "bot",
    "Hola! Soy tu entrenador virtual. Pregúntame por tu rutina, el volumen semanal, la tabla de hipertrofia, tus PRs o menciona cualquier músculo (pecho, espalda, biceps...)."
  );
}