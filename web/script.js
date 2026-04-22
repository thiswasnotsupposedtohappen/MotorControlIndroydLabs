const MAX_STEPS = 16;
const MAX_RPM = 1728;

// State management
let manualDateOverride = false;
let currentProfile = Array.from({ length: MAX_STEPS }, (_, i) =>
({
    motor_rpm: 0,
    duration: 0,
    sound_file: ''
}));

function getRealStepCount() 
{
    let lastRealStep = 0;
    for (let i = 0; i < currentProfile.length; i++) {
        if (currentProfile[i].duration > 0) {
            lastRealStep = i + 1;
        }
    }
    return lastRealStep;
}

// Initialize UI
document.addEventListener('DOMContentLoaded', () => 
{
    if (document.getElementById('profileBody')) {
        generateProfileRows();
    }
    setupEventListeners();
    
    // Initial request for settings and COM ports
    postToCpp({ type: 'GET_INITIAL_DATA' });
    
    // If we are on the reports page, request the reports
    if (document.getElementById('reportsTableBody')) {
        postToCpp({ type: 'GET_REPORTS' });
    }

    // Load persisted date if available
    const savedDate = localStorage.getItem('selectedDate');
    if (savedDate) {
        manualDateOverride = true;
        updateDate(new Date(savedDate));
    } else {
        updateDate();
    }

    // Start date updater
    const dateInterval = setInterval(() => {
        if (!manualDateOverride) {
            updateDate();
        } else {
            clearInterval(dateInterval);
        }
    }, 60000); // Every minute
});

function updateDate(dateObj = new Date()) 
{
    const days = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
    const dayName = days[dateObj.getDay()];
    const dateStr = dateObj.toLocaleDateString('en-GB'); // dd/mm/yyyy

    const dayEl = document.getElementById('displayDay');
    const dateEl = document.getElementById('displayDate');
    const pickerEl = document.getElementById('datePicker');

    if (dayEl) dayEl.textContent = dayName;
    if (dateEl) dateEl.textContent = dateStr;
    
    if (pickerEl) {
        // format as yyyy-mm-dd for input
        const yyyy = dateObj.getFullYear();
        const mm = String(dateObj.getMonth() + 1).padStart(2, '0');
        const dd = String(dateObj.getDate()).padStart(2, '0');
        pickerEl.value = `${yyyy}-${mm}-${dd}`;
    }
}

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

    // Configuration (Buttons)
    const seatsGroup = document.getElementById('seatsGroup');
    if (seatsGroup) {
        seatsGroup.addEventListener('click', (e) => {
            const btn = e.target.closest('.btn-toggle');
            if (btn) {
                const val = parseInt(btn.dataset.value);
                setActiveInGroup(seatsGroup, val);
                postToCpp({ type: 'SET_SEATS', value: val });
            }
        });
    }

    const difficultyGroup = document.getElementById('difficultyGroup');
    if (difficultyGroup) {
        difficultyGroup.addEventListener('click', (e) => {
            const btn = e.target.closest('.btn-toggle');
            if (btn) {
                const val = parseInt(btn.dataset.value);
                setActiveInGroup(difficultyGroup, val);
                if (document.getElementById('profileTable')) {
                    readProfileFromUI();
                }
                postToCpp({ type: 'SET_DIFFICULTY', value: val });
            }
        });
    }

    const datePicker = document.getElementById('datePicker');
    if (datePicker) {
        datePicker.addEventListener('change', (e) => {
            if (e.target.value) {
                manualDateOverride = true;
                localStorage.setItem('selectedDate', e.target.value);
                const newDate = new Date(e.target.value);
                updateDate(newDate);
            }
        });

        // Explicitly trigger the picker when the card is clicked
        const dateCard = document.getElementById('dateCard');
        if (dateCard) {
            dateCard.addEventListener('click', () => {
                if (typeof datePicker.showPicker === 'function') {
                    datePicker.showPicker();
                } else {
                    datePicker.click();
                }
            });
        }
    }

    // Control
    const btnStart = document.getElementById('btnStart');
    if (btnStart) {
        btnStart.addEventListener('click', () => {
            if (document.getElementById('profileTable')) {
                readProfileFromUI();
            }
            let dateStr = "";
            const savedDate = localStorage.getItem('selectedDate');
            if (savedDate) {
                const d = new Date(savedDate);
                const yyyy = d.getFullYear();
                const mm = String(d.getMonth() + 1).padStart(2, '0');
                const dd = String(d.getDate()).padStart(2, '0');
                dateStr = `${dd}/${mm}/${yyyy}`;
            } else {
                const d = new Date();
                const yyyy = d.getFullYear();
                const mm = String(d.getMonth() + 1).padStart(2, '0');
                const dd = String(d.getDate()).padStart(2, '0');
                dateStr = `${dd}/${mm}/${yyyy}`;
            }
            postToCpp({ type: 'START', profile: currentProfile, date: dateStr });
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

function setActiveInGroup(group, value) 
{
    const buttons = group.querySelectorAll('.btn-toggle');
    buttons.forEach(btn => {
        if (parseInt(btn.dataset.value) === value) {
            btn.classList.add('active');
        } else {
            btn.classList.remove('active');
        }
    });
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
            updateProfileUI(msg.profile, msg.difficulty, msg.seats, msg.total_rides);
            break;
        case 'SOUND_SELECTED':
            updateSoundFile(msg.index, msg.path, msg.filename);
            break;
        case 'REPORTS_DATA':
            renderReports(msg.logs);
            break;
    }
}

function renderReports(logs) {
    const tbody = document.getElementById('reportsTableBody');
    if (!tbody) return;
    
    tbody.innerHTML = '';
    
    if (logs.length === 0) {
        tbody.innerHTML = '<tr><td colspan="3" style="text-align:center;">No logs found</td></tr>';
        return;
    }
    
    // Display in reverse chronological order
    for (let i = logs.length - 1; i >= 0; i--) {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${logs[i].date}</td>
            <td>${logs[i].time}</td>
            <td>${logs[i].people}</td>
        `;
        tbody.appendChild(tr);
    }
}

function updateTotalRides(count) 
{
    const el = document.getElementById('totalRides');
    if (!el) return;
    
    // Convert to string and ensure at least 3 digits
    const str = count.toString().padStart(3, '0');
    el.innerHTML = '';
    for (const char of str) {
        const span = document.createElement('span');
        span.textContent = char;
        el.appendChild(span);
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
    if (statusStep) {
        const total = getRealStepCount();
        const current = data.running ? (data.currentStep + 1) : 0;
        statusStep.textContent = `${current} / ${total}`;
    }

    const btnStart = document.getElementById('btnStart');
    if (btnStart) btnStart.disabled = data.running;
    
    const btnStop = document.getElementById('btnStop');
    if (btnStop) btnStop.disabled = !data.running;

    const btnBack = document.getElementById('btnBack');
    if (btnBack) {
        if (data.running) {
            btnBack.classList.add('disabled');
        } else {
            btnBack.classList.remove('disabled');
        }
    }
    
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

function updateProfileUI(profile, difficulty, seats, totalRides) 
{
    const difficultyGroup = document.getElementById('difficultyGroup');
    if (difficultyGroup) setActiveInGroup(difficultyGroup, difficulty);
    
    const seatsGroup = document.getElementById('seatsGroup');
    if (seatsGroup) setActiveInGroup(seatsGroup, seats);
    
    if (totalRides !== undefined) updateTotalRides(totalRides);
    
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
