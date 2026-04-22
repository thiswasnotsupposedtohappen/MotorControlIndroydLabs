const MAX_STEPS = 16;
const MAX_RPM = 1728;

// State management
let currentProfile = Array.from({ length: MAX_STEPS }, (_, i) => ({
    motor_rpm: 0,
    duration: 0,
    sound_file: ''
}));

// Initialize UI
document.addEventListener('DOMContentLoaded', () => {
    generateProfileRows();
    setupEventListeners();
    
    // Initial request for settings and COM ports
    postToCpp({ type: 'GET_INITIAL_DATA' });
});

function generateProfileRows() {
    const tbody = document.getElementById('profileBody');
    tbody.innerHTML = '';

    for (let i = 0; i < MAX_STEPS; i++) {
        const tr = document.createElement('tr');
        tr.id = `step-row-${i}`;
        tr.innerHTML = `
            <td class="step-no">${i + 1}</td>
            <td><input type="number" class="table-input rpm-input" value="0" min="0" max="${MAX_RPM}" data-index="${i}"></td>
            <td><input type="number" class="table-input dur-input" value="0" min="0" max="600" data-index="${i}"></td>
            <td class="abs-time" id="abs-time-${i}">0</td>
            <td>
                <div class="sound-file-container">
                    <input type="text" class="table-input snd-input" readonly placeholder="No file selected" id="snd-file-${i}">
                    <button class="btn icon btn-browse" data-index="${i}">...</button>
                </div>
            </td>
            <td><button class="btn icon btn-clear" data-index="${i}">✕</button></td>
        `;
        tbody.appendChild(tr);
    }
}

function setupEventListeners() {
    // Connection
    document.getElementById('btnConnect').addEventListener('click', () => {
        const port = document.getElementById('comPort').value;
        postToCpp({ type: 'CONNECT', port: port });
    });

    document.getElementById('btnRefresh').addEventListener('click', () => {
        postToCpp({ type: 'REFRESH_PORTS' });
    });

    // Configuration
    document.getElementById('numSeats').addEventListener('change', (e) => {
        postToCpp({ type: 'SET_SEATS', value: parseInt(e.target.value) });
    });

    document.getElementById('difficulty').addEventListener('change', (e) => {
        readProfileFromUI();
        postToCpp({ type: 'SET_DIFFICULTY', value: parseInt(e.target.value) });
    });

    // Control
    document.getElementById('btnStart').addEventListener('click', () => {
        readProfileFromUI();
        postToCpp({ type: 'START', profile: currentProfile });
    });

    document.getElementById('btnStop').addEventListener('click', () => {
        postToCpp({ type: 'STOP' });
    });

    // File Actions
    document.getElementById('btnSave').addEventListener('click', () => {
        readProfileFromUI();
        postToCpp({ type: 'SAVE', profile: currentProfile });
    });

    document.getElementById('btnLoad').addEventListener('click', () => {
        postToCpp({ type: 'LOAD' });
    });

    // Table Events (Delegation)
    document.getElementById('profileTable').addEventListener('input', (e) => {
        if (e.target.classList.contains('dur-input')) {
            updateAbsoluteTimes();
        }
    });

    document.getElementById('profileTable').addEventListener('click', (e) => {
        if (e.target.classList.contains('btn-browse')) {
            const index = e.target.dataset.index;
            postToCpp({ type: 'BROWSE_SOUND', index: parseInt(index) });
        }
        if (e.target.classList.contains('btn-clear')) {
            const index = e.target.dataset.index;
            clearStep(index);
        }
    });
}

function readProfileFromUI() {
    const rpms = document.querySelectorAll('.rpm-input');
    const durs = document.querySelectorAll('.dur-input');
    const snds = document.querySelectorAll('.snd-input');

    for (let i = 0; i < MAX_STEPS; i++) {
        currentProfile[i].motor_rpm = parseInt(rpms[i].value) || 0;
        currentProfile[i].duration = parseInt(durs[i].value) || 0;
        // sound_file is managed by Browse message, we don't read it from the readonly text field
    }
}

function updateAbsoluteTimes() {
    const durs = document.querySelectorAll('.dur-input');
    let total = 0;
    durs.forEach((input, i) => {
        total += parseInt(input.value) || 0;
        document.getElementById(`abs-time-${i}`).textContent = total;
    });
}

function clearStep(index) {
    const row = document.getElementById(`step-row-${index}`);
    row.querySelector('.rpm-input').value = 0;
    row.querySelector('.dur-input').value = 0;
    row.querySelector('.snd-input').value = '';
    currentProfile[index].sound_file = '';
    updateAbsoluteTimes();
}

// Communication with C++
function postToCpp(data) {
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(data);
    } else {
        console.log('WebView2 not available. Message:', data);
    }
}

// Handle messages from C++
if (window.chrome && window.chrome.webview) {
    window.chrome.webview.addEventListener('message', event => {
        const msg = event.data;
        handleCppMessage(msg);
    });
}

function handleCppMessage(msg) {
    switch (msg.type) {
        case 'UPDATE_TELEMETRY':
            updateTelemetry(msg.data);
            break;
        case 'UPDATE_STATUS':
            updateStatus(msg.data);
            break;
        case 'COM_PORTS':
            updateComPorts(msg.ports, msg.selected);
            break;
        case 'LOAD_PROFILE':
            updateProfileUI(msg.profile, msg.difficulty, msg.seats);
            break;
        case 'SOUND_SELECTED':
            updateSoundFile(msg.index, msg.path, msg.filename);
            break;
    }
}

function updateTelemetry(data) {
    // RPM
    document.getElementById('valRpm').textContent = data.rpm.toFixed(2);
    document.getElementById('gaugeRpm').style.width = `${(data.rpm / MAX_RPM) * 100}%`;
    
    // Current
    document.getElementById('valCurrent').textContent = data.current.toFixed(2);
    document.getElementById('gaugeCurrent').style.width = `${Math.min(data.current * 20, 100)}%`; // Scaled for display
    
    // Voltage
    document.getElementById('valVoltage').textContent = data.voltage.toFixed(2);
    document.getElementById('gaugeVoltage').style.width = `${(data.voltage / 440) * 100}%`;
}

function updateStatus(data) {
    const connEl = document.getElementById('statusConn');
    connEl.textContent = data.connected ? 'CONNECTED' : 'DISCONNECTED';
    connEl.className = `status-indicator ${data.connected ? 'connected' : 'disconnected'}`;

    const progEl = document.getElementById('statusProg');
    progEl.textContent = data.running ? 'RUNNING' : 'STOPPED';
    progEl.className = `status-indicator ${data.running ? 'running' : 'stopped'}`;

    document.getElementById('statusTime').textContent = `${data.elapsedTime} s`;
    document.getElementById('statusStep').textContent = `${data.currentStep + 1} / 16`;

    document.getElementById('btnStart').disabled = data.running;
    document.getElementById('btnStop').disabled = !data.running;
    document.getElementById('btnConnect').textContent = data.connected ? 'DISCONNECT' : 'CONNECT';
    
    // Highlight active step
    document.querySelectorAll('tr').forEach(tr => tr.classList.remove('active-step'));
    if (data.running) {
        document.getElementById(`step-row-${data.currentStep}`).classList.add('active-step');
    }
}

function updateComPorts(ports, selected) {
    const select = document.getElementById('comPort');
    select.innerHTML = '';
    ports.forEach(port => {
        const opt = document.createElement('option');
        opt.value = port.name;
        opt.textContent = port.friendly;
        if (port.name === selected) opt.selected = true;
        select.appendChild(opt);
    });
}

function updateProfileUI(profile, difficulty, seats) {
    document.getElementById('difficulty').value = difficulty;
    document.getElementById('numSeats').value = seats;
    
    currentProfile = profile;
    const rpms = document.querySelectorAll('.rpm-input');
    const durs = document.querySelectorAll('.dur-input');
    const snds = document.querySelectorAll('.snd-input');

    profile.forEach((step, i) => {
        rpms[i].value = step.motor_rpm;
        durs[i].value = step.duration;
        snds[i].value = step.sound_file.split('\\').pop() || '';
    });
    updateAbsoluteTimes();
}

function updateSoundFile(index, path, filename) {
    currentProfile[index].sound_file = path;
    document.getElementById(`snd-file-${index}`).value = filename;
}
