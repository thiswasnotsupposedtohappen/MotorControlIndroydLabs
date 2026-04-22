const MAX_STEPS = 16;
const MAX_RPM = 1728;

// State management
let currentProfile = Array.from({ length: MAX_STEPS }, (_, i) =>
({
    motor_rpm: 0,
    duration: 0,
    sound_file: ''
}));

// Initialize UI
document.addEventListener('DOMContentLoaded', () => 
{
    if (document.getElementById('profileBody')) {
        generateProfileRows();
    }
    setupEventListeners();
    
    // Initial request for settings and COM ports
    postToCpp({ type: 'GET_INITIAL_DATA' });
});

function generateProfileRows() 
{
    const tbody = document.getElementById('profileBody');
    tbody.innerHTML = '';

    for (let i = 0; i < MAX_STEPS; i++) 
    {
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

function setupEventListeners() 
{
    // Connection
    const btnConnect = document.getElementById('btnConnect');
    if (btnConnect) {
        btnConnect.addEventListener('click', () => {
            const port = document.getElementById('comPort').value;
            postToCpp({ type: 'CONNECT', port: port });
        });
    }

    const btnRefresh = document.getElementById('btnRefresh');
    if (btnRefresh) {
        btnRefresh.addEventListener('click', () => {
            postToCpp({ type: 'REFRESH_PORTS' });
        });
    }

    // Configuration
    const numSeats = document.getElementById('numSeats');
    if (numSeats) {
        numSeats.addEventListener('change', (e) => {
            postToCpp({ type: 'SET_SEATS', value: parseInt(e.target.value) });
        });
    }

    const difficulty = document.getElementById('difficulty');
    if (difficulty) {
        difficulty.addEventListener('change', (e) => {
            readProfileFromUI();
            postToCpp({ type: 'SET_DIFFICULTY', value: parseInt(e.target.value) });
        });
    }

    // Control
    const btnStart = document.getElementById('btnStart');
    if (btnStart) {
        btnStart.addEventListener('click', () => {
            if (document.getElementById('profileTable')) {
                readProfileFromUI();
            }
            postToCpp({ type: 'START', profile: currentProfile });
        });
    }

    const btnStop = document.getElementById('btnStop');
    if (btnStop) {
        btnStop.addEventListener('click', () => {
            postToCpp({ type: 'STOP' });
        });
    }

    // Table Events (Delegation)
    const profileTable = document.getElementById('profileTable');
    if (profileTable) {
        profileTable.addEventListener('input', (e) => {
            if (e.target.classList.contains('dur-input')) {
                updateAbsoluteTimes();
            }
        });

        profileTable.addEventListener('change', (e) => {
            if (e.target.classList.contains('rpm-input') || e.target.classList.contains('dur-input')) {
                const index = parseInt(e.target.dataset.index);
                const value = parseInt(e.target.value) || 0;
                const field = e.target.classList.contains('rpm-input') ? 'rpm' : 'duration';
                postToCpp({ type: 'UPDATE_STEP', index: index, field: field, value: value });
            }
        });

        profileTable.addEventListener('click', (e) => {
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
}

function readProfileFromUI() 
{
    const rpms = document.querySelectorAll('.rpm-input');
    const durs = document.querySelectorAll('.dur-input');
    const snds = document.querySelectorAll('.snd-input');

    for (let i = 0; i < MAX_STEPS; i++) 
    {
        currentProfile[i].motor_rpm = parseInt(rpms[i].value) || 0;
        currentProfile[i].duration = parseInt(durs[i].value) || 0;
        // sound_file is managed by Browse message, we don't read it from the readonly text field
    }
}

function updateAbsoluteTimes() 
{
    const durs = document.querySelectorAll('.dur-input');
    let total = 0;
    durs.forEach((input, i) => 
    {
        total += parseInt(input.value) || 0;
        document.getElementById(`abs-time-${i}`).textContent = total;
    });
}

function clearStep(index) 
{
    const row = document.getElementById(`step-row-${index}`);
    row.querySelector('.rpm-input').value = 0;
    row.querySelector('.dur-input').value = 0;
    row.querySelector('.snd-input').value = '';
    currentProfile[index].sound_file = '';
    updateAbsoluteTimes();
}

// Communication with C++
function postToCpp(data) 
{
    if (window.chrome && window.chrome.webview) 
    {
        window.chrome.webview.postMessage(data);
    }
    else 
    {
        console.log('WebView2 not available. Message:', data);
    }
}

// Handle messages from C++
if (window.chrome && window.chrome.webview) 
{
    window.chrome.webview.addEventListener('message', event => 
    {
        const msg = event.data;
        handleCppMessage(msg);
    });
}

function handleCppMessage(msg) 
{
    switch (msg.type) 
    {
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

function updateTelemetry(data) 
{
    // RPM
    const valRpm = document.getElementById('valRpm');
    if (valRpm) valRpm.textContent = data.rpm.toFixed(2);
    // const gaugeRpm = document.getElementById('gaugeRpm');
    // if (gaugeRpm) gaugeRpm.style.width = `${(data.rpm / MAX_RPM) * 100}%`;
    
    // Current
    const valCurrent = document.getElementById('valCurrent');
    if (valCurrent) valCurrent.textContent = data.current.toFixed(2);
    // const gaugeCurrent = document.getElementById('gaugeCurrent');
    // if (gaugeCurrent) gaugeCurrent.style.width = `${Math.min(data.current * 20, 100)}%`; // Scaled for display
    
    // Voltage
    const valVoltage = document.getElementById('valVoltage');
    if (valVoltage) valVoltage.textContent = data.voltage.toFixed(2);
    // const gaugeVoltage = document.getElementById('gaugeVoltage');
    // if (gaugeVoltage) gaugeVoltage.style.width = `${(data.voltage / 440) * 100}%`;
}

function updateStatus(data) 
{
    const connEl = document.getElementById('statusConn');
    if (connEl) {
        connEl.textContent = data.connected ? 'CONNECTED' : 'DISCONNECTED';
        connEl.className = `status-indicator ${data.connected ? 'connected' : 'disconnected'}`;
    }

    const progEl = document.getElementById('statusProg');
    if (progEl) {
        progEl.textContent = data.running ? 'RUNNING' : 'STOPPED';
        progEl.className = `status-indicator ${data.running ? 'running' : 'stopped'}`;
    }

    const statusTime = document.getElementById('statusTime');
    if (statusTime) statusTime.textContent = `${data.elapsedTime} s`;

    const bigTimer = document.getElementById('bigTimer');
    if (bigTimer) {
        const mins = Math.floor(data.elapsedTime / 60);
        const secs = data.elapsedTime % 60;
        bigTimer.textContent = `${mins.toString().padStart(2, '0')}:${secs.toString().padStart(2, '0')}`;
    }

    const statusStep = document.getElementById('statusStep');
    if (statusStep) statusStep.textContent = `${data.currentStep + 1} / 16`;

    const btnStart = document.getElementById('btnStart');
    if (btnStart) btnStart.disabled = data.running;
    
    const btnStop = document.getElementById('btnStop');
    if (btnStop) btnStop.disabled = !data.running;
    
    const btnConnect = document.getElementById('btnConnect');
    if (btnConnect) btnConnect.textContent = data.connected ? 'DISCONNECT' : 'CONNECT';
    
    // Highlight active step
    document.querySelectorAll('tr').forEach(tr => tr.classList.remove('active-step'));
    if (data.running) 
    {
        const activeRow = document.getElementById(`step-row-${data.currentStep}`);
        if (activeRow) activeRow.classList.add('active-step');
    }
}

function updateComPorts(ports, selected) 
{
    const select = document.getElementById('comPort');
    if (!select) return;
    
    select.innerHTML = '';
    ports.forEach(port => 
    {
        const opt = document.createElement('option');
        opt.value = port.name;
        opt.textContent = port.friendly;
        if (port.name === selected) opt.selected = true;
        select.appendChild(opt);
    });
}

function updateProfileUI(profile, difficulty, seats) 
{
    const difficultyEl = document.getElementById('difficulty');
    if (difficultyEl) difficultyEl.value = difficulty;
    
    const numSeatsEl = document.getElementById('numSeats');
    if (numSeatsEl) numSeatsEl.value = seats;
    
    currentProfile = profile;
    const rpms = document.querySelectorAll('.rpm-input');
    const durs = document.querySelectorAll('.dur-input');
    const snds = document.querySelectorAll('.snd-input');

    if (rpms.length > 0) {
        profile.forEach((step, i) => 
        {
            rpms[i].value = step.motor_rpm;
            durs[i].value = step.duration;
            snds[i].value = step.sound_file.split('\\').pop() || '';
        });
        updateAbsoluteTimes();
    }
}

function updateSoundFile(index, path, filename) 
{
    currentProfile[index].sound_file = path;
    document.getElementById(`snd-file-${index}`).value = filename;
}
