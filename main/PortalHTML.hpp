#pragma once

#include <cstddef>

constexpr const char* PORTAL_HTML = R"rawliteral(
<!DOCTYPE html>
<html lang="he" dir="rtl">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>AlertLight הגדרות</title>
    <style>
        :root {
            --bg: #f5f5f7;
            --surface: #ffffff;
            --primary: #007aff;
            --primary-dark: #0056b3;
            --success: #34c759;
            --error: #ff3b30;
            --warning: #ff9500;
            --text: #1c1c1e;
            --text-secondary: #8e8e93;
            --border: #e5e5ea;
            --radius: 12px;
            --shadow: 0 2px 8px rgba(0,0,0,0.08);
        }
        
        [data-theme="dark"] {
            --bg: #1c1c1e;
            --surface: #2c2c2e;
            --primary: #0a84ff;
            --primary-dark: #409cff;
            --success: #30d158;
            --error: #ff453a;
            --warning: #ff9f0a;
            --text: #f5f5f7;
            --text-secondary: #8e8e93;
            --border: #38383a;
            --shadow: 0 2px 8px rgba(0,0,0,0.3);
        }
        
        * { box-sizing: border-box; -webkit-tap-highlight-color: transparent; }
        
        body {
            margin: 0; padding: 0;
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: var(--bg);
            color: var(--text);
            padding-bottom: 80px;
            min-height: 100vh;
            transition: background 0.3s, color 0.3s;
        }
        
        .header {
            background: var(--surface);
            padding: 20px;
            text-align: center;
            box-shadow: var(--shadow);
            position: sticky;
            top: 0;
            z-index: 100;
            transition: background 0.3s;
        }
        
        .header h1 { margin: 0; font-size: 20px; font-weight: 600; }
        
        .content { padding: 20px; }
        
        .card {
            background: var(--surface);
            border-radius: var(--radius);
            padding: 20px;
            margin-bottom: 16px;
            box-shadow: var(--shadow);
            transition: background 0.3s;
        }
        
        .card-title {
            font-size: 14px;
            font-weight: 600;
            color: var(--text-secondary);
            margin-bottom: 12px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        .input-group { margin-bottom: 16px; }
        
        .input-group label {
            display: block;
            font-size: 14px;
            font-weight: 500;
            margin-bottom: 8px;
        }
        
        input, select {
            width: 100%;
            padding: 14px 16px;
            font-size: 16px;
            border: 1px solid var(--border);
            border-radius: var(--radius);
            background: var(--bg);
            color: var(--text);
            transition: border-color 0.2s, background 0.3s;
        }
        
        input:focus, select:focus {
            outline: none;
            border-color: var(--primary);
        }
        
        .btn {
            width: 100%;
            padding: 16px;
            font-size: 17px;
            font-weight: 600;
            border: none;
            border-radius: var(--radius);
            cursor: pointer;
            transition: opacity 0.2s, transform 0.1s;
        }
        
        .btn:active { transform: scale(0.98); }
        
        .btn-primary {
            background: var(--primary);
            color: white;
        }
        
        .btn-primary:disabled {
            opacity: 0.6;
            cursor: not-allowed;
        }
        
        .btn-secondary {
            background: var(--bg);
            color: var(--primary);
            border: 2px solid var(--primary);
        }
        
        .btn-danger {
            background: var(--error);
            color: white;
        }
        
        .btn-small {
            width: auto;
            padding: 10px 16px;
            font-size: 14px;
        }
        
        .tabs {
            position: fixed;
            bottom: 0;
            left: 0;
            right: 0;
            background: var(--surface);
            display: flex;
            border-top: 1px solid var(--border);
            padding-bottom: env(safe-area-inset-bottom);
            z-index: 100;
            transition: background 0.3s;
        }
        
        .tab {
            flex: 1;
            padding: 12px 8px;
            text-align: center;
            font-size: 11px;
            font-weight: 500;
            color: var(--text-secondary);
            cursor: pointer;
            transition: color 0.2s;
            border: none;
            background: none;
        }
        
        .tab.active { color: var(--primary); }
        
        .tab-icon {
            display: block;
            font-size: 24px;
            margin-bottom: 4px;
        }
        
        .view { display: none; }
        .view.active { display: block; }
        
        .toast {
            position: fixed;
            top: 20px;
            left: 20px;
            right: 20px;
            padding: 16px 20px;
            border-radius: var(--radius);
            color: white;
            font-weight: 500;
            z-index: 1000;
            transform: translateY(-100px);
            transition: transform 0.3s ease;
            text-align: center;
        }
        
        .toast.show { transform: translateY(0); }
        
        .toast.success { background: var(--success); }
        .toast.error { background: var(--error); }
        .toast.warning { background: var(--warning); }
        
        .chip {
            display: inline-flex;
            align-items: center;
            padding: 8px 12px;
            background: var(--primary);
            color: white;
            border-radius: 20px;
            font-size: 14px;
            margin: 4px;
        }
        
        .chip-remove {
            margin-right: 8px;
            cursor: pointer;
            font-size: 18px;
            line-height: 1;
        }
        
        .city-list {
            max-height: 300px;
            overflow-y: auto;
            border: 1px solid var(--border);
            border-radius: var(--radius);
        }
        
        .city-item {
            padding: 12px 16px;
            border-bottom: 1px solid var(--border);
            cursor: pointer;
            transition: background 0.2s;
        }
        
        .city-item:last-child { border-bottom: none; }
        .city-item:hover { background: var(--bg); }
        .city-item.selected { background: var(--primary); color: white; }
        
        .sequence-step {
            display: flex;
            align-items: center;
            gap: 12px;
            padding: 12px;
            background: var(--bg);
            border-radius: var(--radius);
            margin-bottom: 12px;
            flex-wrap: wrap;
        }
        
        .sequence-step select {
            flex: 1;
            min-width: 120px;
            padding: 10px;
        }
        
        .sequence-step .remove {
            padding: 8px;
            color: var(--error);
            font-size: 20px;
            background: none;
            border: none;
            cursor: pointer;
        }
        
        .template-btns {
            display: flex;
            gap: 8px;
            flex-wrap: wrap;
            margin-bottom: 16px;
        }
        
        .template-btn {
            padding: 10px 16px;
            font-size: 14px;
            background: var(--bg);
            border: none;
            border-radius: 20px;
            cursor: pointer;
            color: var(--text);
        }
        
        .alert-type-toggle {
            display: flex;
            background: var(--bg);
            border-radius: var(--radius);
            padding: 4px;
            margin-bottom: 16px;
        }
        
        .alert-type-btn {
            flex: 1;
            padding: 12px;
            border: none;
            background: transparent;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 500;
            cursor: pointer;
            transition: background 0.2s;
            color: var(--text);
        }
        
        .alert-type-btn.active {
            background: var(--surface);
            box-shadow: var(--shadow);
        }
        
        .status-indicator {
            display: inline-flex;
            align-items: center;
            gap: 8px;
        }
        
        .status-dot {
            width: 10px;
            height: 10px;
            border-radius: 50%;
        }
        
        .status-dot.green { background: var(--success); }
        .status-dot.red { background: var(--error); }
        
        .log-viewer {
            background: #1e1e1e;
            color: #d4d4d4;
            font-family: 'SF Mono', Monaco, monospace;
            font-size: 12px;
            padding: 12px;
            border-radius: var(--radius);
            max-height: 300px;
            overflow-y: auto;
            white-space: pre-wrap;
            word-break: break-all;
        }
        
        .modal-overlay {
            display: none;
            position: fixed;
            top: 0; left: 0; right: 0; bottom: 0;
            background: rgba(0,0,0,0.5);
            z-index: 200;
            align-items: center;
            justify-content: center;
        }
        
        .modal-overlay.show { display: flex; }
        
        .modal {
            background: var(--surface);
            border-radius: var(--radius);
            padding: 24px;
            margin: 20px;
            max-width: 400px;
            width: 100%;
            text-align: center;
            transition: background 0.3s;
        }
        
        .modal h3 { margin: 0 0 12px; }
        
        .spinner {
            width: 40px;
            height: 40px;
            border: 3px solid var(--border);
            border-top-color: var(--primary);
            border-radius: 50%;
            animation: spin 1s linear infinite;
            margin: 20px auto;
        }
        
        @keyframes spin { to { transform: rotate(360deg); } }
        
        .form-row {
            display: flex;
            gap: 12px;
            align-items: center;
        }
        
        .form-row .btn { width: auto; flex: 1; }
        
        .password-toggle {
            position: relative;
        }
        
        .password-toggle input { padding-right: 50px; }
        
        .password-toggle .toggle {
            position: absolute;
            right: 16px;
            top: 50%;
            transform: translateY(-50%);
            background: none;
            border: none;
            font-size: 18px;
            cursor: pointer;
        }
        
        /* Slider Styles */
        .slider-container {
            display: flex;
            align-items: center;
            gap: 12px;
            width: 100%;
        }
        
        .slider-container input[type="range"] {
            flex: 1;
            padding: 0;
            border: none;
            background: transparent;
            -webkit-appearance: none;
            height: 30px;
        }
        
        .slider-container input[type="range"]::-webkit-slider-track {
            height: 8px;
            background: var(--primary);
            border-radius: 4px;
            opacity: 0.5;
        }
        
        .slider-container input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 24px;
            height: 24px;
            background: var(--primary);
            border-radius: 50%;
            cursor: pointer;
            margin-top: -8px;
            border: 2px solid white;
            box-shadow: 0 2px 4px rgba(0,0,0,0.3);
        }
        
        .slider-value {
            min-width: 50px;
            text-align: center;
            font-size: 14px;
            font-weight: 500;
            color: var(--primary);
        }
        
        /* Color Select */
        .color-select-container {
            display: flex;
            align-items: center;
            gap: 8px;
        }
        
        .color-select-container select {
            padding: 8px;
            font-size: 14px;
        }
        
        /* Remove Button */
        .sequence-step .remove {
            padding: 8px 12px;
            color: white;
            font-size: 14px;
            font-weight: 600;
            background: var(--error);
            border: none;
            border-radius: 8px;
            cursor: pointer;
            min-width: 60px;
        }
        
        .sequence-step .remove:hover {
            opacity: 0.8;
        }
        
        /* Toggle Switch */
        .toggle-switch {
            position: relative;
            width: 51px;
            height: 31px;
        }
        
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        
        .toggle-slider {
            position: absolute;
            cursor: pointer;
            top: 0; left: 0; right: 0; bottom: 0;
            background-color: var(--border);
            transition: 0.3s;
            border-radius: 31px;
        }
        
        .toggle-slider:before {
            position: absolute;
            content: "";
            height: 27px;
            width: 27px;
            left: 2px;
            bottom: 2px;
            background-color: white;
            transition: 0.3s;
            border-radius: 50%;
        }
        
        input:checked + .toggle-slider {
            background-color: var(--primary);
        }
        
        input:checked + .toggle-slider:before {
            transform: translateX(20px);
        }
        
        .setting-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 12px 0;
            border-bottom: 1px solid var(--border);
        }
        
        .setting-row:last-child { border-bottom: none; }
        
        .setting-label {
            font-size: 16px;
            font-weight: 500;
        }
        
        .selected-cities-mini {
            display: flex;
            flex-wrap: wrap;
            gap: 8px;
            margin-bottom: 12px;
        }
        
        .city-chip-mini {
            padding: 6px 12px;
            background: var(--primary);
            color: white;
            border-radius: 16px;
            font-size: 13px;
        }
        
        .empty-state {
            text-align: center;
            padding: 20px;
            color: var(--text-secondary);
        }
        
        .action-icon {
            font-size: 20px;
            margin-right: 8px;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>🔔 AlertLight הגדרות</h1>
    </div>
    
    <div class="content">
        <!-- WiFi View -->
        <div id="view-wifi" class="view active">
            <div class="card">
                <div class="card-title">📶 סטטוס חיבור</div>
                <div id="wifi-status" class="status-indicator">
                    <span class="status-dot red"></span>
                    <span>לא מחובר</span>
                </div>
            </div>
            
            <div class="card">
                <div class="card-title">📡 הגדרת רשת</div>
                <div class="input-group">
                    <label>🔗 שם הרשת (SSID)</label>
                    <input type="text" id="ssid" placeholder="שם הרשת שלכם">
                </div>
                <div class="input-group">
                    <label>🔒 סיסמה</label>
                    <div class="password-toggle">
                        <input type="password" id="password" placeholder="סיסמת הרשת">
                        <button type="button" class="toggle" onclick="togglePassword()">👁</button>
                    </div>
                </div>
                <button class="btn btn-primary" id="connect-btn" onclick="connectWiFi()">
                    💾 שמור והתחבר
                </button>
            </div>
        </div>
        
        <!-- LED View -->
        <div id="view-led" class="view">
            <div class="card">
                <div class="card-title">🏙️ ערים להתראה</div>
                <div id="selected-cities-mini" class="selected-cities-mini"></div>
                <button class="btn btn-secondary btn-small" onclick="showTab('cities')">
                    ➕ הוסף ערים
                </button>
            </div>
            
            <div class="card">
                <div class="card-title">⚠️ סוג התראה</div>
                <div class="alert-type-toggle">
                    <button class="alert-type-btn active" onclick="setAlertType('warning')">
                        ⚠️ אזהרה
                    </button>
                    <button class="alert-type-btn" onclick="setAlertType('rocket')">
                        🚀 רקטות
                    </button>
                </div>
            </div>
            
            <div class="card">
                <div class="card-title">💡 בניית רצף</div>
                
                <div class="template-btns">
                    <button class="template-btn" onclick="applyTemplate('blink')">💥 הבהוב</button>
                    <button class="template-btn" onclick="applyTemplate('pulse')">💫 פולס</button>
                    <button class="template-btn" onclick="applyTemplate('warning')">⚠️ אזהרה</button>
                </div>
                
                <div id="sequence-steps"></div>
                
                <button class="btn btn-secondary btn-small" onclick="addStep()">
                    ➕ הוסף פעולה
                </button>
            </div>
            
            <div class="card">
                <div class="card-title">⏱️ משך ההתראה</div>
                <select id="alert-duration" onchange="saveDuration()">
                    <option value="30">30 שניות</option>
                    <option value="60">דקה</option>
                    <option value="120" selected>2 דקות</option>
                    <option value="300">5 דקות</option>
                    <option value="0">🔴 עד סוף האירוע</option>
                </select>
            </div>
            
            <button class="btn btn-primary" onclick="saveSequence()">
                💾 שמור רצף
            </button>
        </div>
        
        <!-- Cities View -->
        <div id="view-cities" class="view">
            <div class="card">
                <div class="card-title">🏙️ בחירת ערים</div>
                <div class="input-group">
                    <input type="text" id="city-search" placeholder="🔍 חיפוש עיר..." oninput="filterCities()">
                </div>
                <div class="city-list" id="city-list" style="max-height: 400px;"></div>
            </div>
            
            <div class="card">
                <div class="card-title">✅ ערים נבחרו (<span id="selected-count">0</span>)</div>
                <div id="selected-cities-list"></div>
            </div>
            
            <button class="btn btn-primary" onclick="saveCities()">
                💾 שמור ערים
            </button>
        </div>
        
        <!-- IR View -->
        <div id="view-ir" class="view">
            <div class="card">
                <div class="card-title">📱 פקודות IR</div>
                <div id="ir-commands"></div>
            </div>
        </div>
        
        <!-- System View -->
        <div id="view-system" class="view">
            <div class="card">
                <div class="card-title">📋 לוג מערכת</div>
                <button class="btn btn-secondary btn-small" onclick="refreshLogs()" style="margin-bottom: 12px;">
                    🔄 רענן
                </button>
                <div class="log-viewer" id="logs">טוען...</div>
            </div>
            
            <div class="card">
                <div class="card-title">🔧 פעולות</div>
                <div style="display: flex; flex-direction: column; gap: 12px;">
                    <button class="btn btn-secondary btn-small" onclick="restart()">
                        🔄 אתחל מערכת
                    </button>
                    <button class="btn btn-secondary btn-small" onclick="checkUpdate()">
                        📦 בדוק עדכונים
                    </button>
                    <button class="btn btn-secondary btn-small" onclick="testLights()">
                        💡 בדיקת אורות
                    </button>
                </div>
            </div>
            
            <div class="card">
                <div class="card-title">⚙️ הגדרות מערכת</div>
                
                <div class="setting-row">
                    <span class="setting-label">🌙 מצב כהה</span>
                    <label class="toggle-switch">
                        <input type="checkbox" id="dark-mode-toggle" onchange="toggleDarkMode()">
                        <span class="toggle-slider"></span>
                    </label>
                </div>
            </div>
        </div>
    </div>
    
    <!-- Tabs -->
    <div class="tabs">
        <button class="tab active" onclick="showTab('wifi')">
            <span class="tab-icon">📶</span>
            <span>WiFi</span>
        </button>
        <button class="tab" onclick="showTab('led')">
            <span class="tab-icon">💡</span>
            <span>LED</span>
        </button>
        <button class="tab" onclick="showTab('cities')">
            <span class="tab-icon">🏙️</span>
            <span>ערים</span>
        </button>
        <button class="tab" onclick="showTab('ir')">
            <span class="tab-icon">📱</span>
            <span>IR</span>
        </button>
        <button class="tab" onclick="showTab('system')">
            <span class="tab-icon">⚙️</span>
            <span>מערכת</span>
        </button>
    </div>
    
    <!-- City Modal (for IR view reference - not used now) -->
    <div class="modal-overlay" id="city-modal">
        <div class="modal">
            <h3>🏙️ בחירת עיר</h3>
            <input type="text" id="city-search-modal" placeholder="🔍 חיפוש..." oninput="filterCities()" style="margin-bottom: 12px;">
            <div class="city-list" id="city-list-modal"></div>
            <button class="btn btn-primary" onclick="closeCityModal()" style="margin-top: 16px;">✅ סיום</button>
        </div>
    </div>
    
    <!-- Record Modal -->
    <div class="modal-overlay" id="record-modal">
        <div class="modal">
            <h3>📱 הקלטת פקודת IR</h3>
            <p>שלח את פקודת ה-IR עכשיו...</p>
            <div class="spinner"></div>
        </div>
    </div>
    
    <!-- Toast -->
    <div class="toast" id="toast"></div>
    
    <script>
        const allCities = [];
        
        let selectedCities = [];
        let alertType = 'warning';
        let sequences = { warning: [], rocket: [] };
        
        // Dark Mode
        function initDarkMode() {
            const saved = localStorage.getItem('darkMode');
            if (saved === 'true' || (!saved && window.matchMedia('(prefers-color-scheme: dark)').matches)) {
                document.documentElement.setAttribute('data-theme', 'dark');
                document.getElementById('dark-mode-toggle').checked = true;
            }
        }
        
        function toggleDarkMode() {
            const isDark = document.getElementById('dark-mode-toggle').checked;
            document.documentElement.setAttribute('data-theme', isDark ? 'dark' : 'light');
            localStorage.setItem('darkMode', isDark);
            showToast(isDark ? '🌙 מצב כהה הופעל' : '☀️ מצב בהיר הופעל', 'success');
        }
        
        function showTab(tab) {
            document.querySelectorAll('.view').forEach(v => v.classList.remove('active'));
            document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
            document.getElementById('view-' + tab).classList.add('active');
            event.target.closest('.tab').classList.add('active');
            
            if (tab === 'system') refreshLogs();
            if (tab === 'cities') loadCities();
        }
        
        function showToast(msg, type = 'success') {
            const toast = document.getElementById('toast');
            toast.textContent = msg;
            toast.className = 'toast ' + type + ' show';
            setTimeout(() => toast.classList.remove('show'), 3000);
        }
        
        function togglePassword() {
            const input = document.getElementById('password');
            input.type = input.type === 'password' ? 'text' : 'password';
        }
        
        // WiFi
        async function connectWiFi() {
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;
            const btn = document.getElementById('connect-btn');
            
            if (!ssid) {
                showToast('נא להזין שם רשת', 'error');
                return;
            }
            
            btn.disabled = true;
            btn.textContent = '🔄 מתחבר...';
            
            try {
                const res = await fetch('/api/wifi/connect', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({ssid, password})
                });
                const data = await res.json();
                
                if (data.success) {
                    showToast('✅ התחברות הצליחה!', 'success');
                    updateWiFiStatus(true, ssid);
                } else {
                    showToast('❌ ' + (data.error || 'שגיאה בהתחברות'), 'error');
                }
            } catch (e) {
                showToast('❌ שגיאה בהתחברות', 'error');
            }
            
            btn.disabled = false;
            btn.textContent = '💾 שמור והתחבר';
        }
        
        async function updateWiFiStatus(connected, ssid) {
            const status = document.getElementById('wifi-status');
            const dot = status.querySelector('.status-dot');
            const text = status.querySelector('span:last-child');
            
            if (connected) {
                dot.className = 'status-dot green';
                text.textContent = ssid + ' - ✅ מחובר';
            } else {
                dot.className = 'status-dot red';
                text.textContent = 'לא מחובר';
            }
        }
        
        // Cities
        async function loadCities() {
            try {
                const res = await fetch('/api/cities');
                const data = await res.json();
                
                if (data.cities && data.cities.length > 0 && allCities.length === 0) {
                    allCities.push(...data.cities);
                }
                
                renderCityList();
                renderSelectedCities();
            } catch (e) {
                console.error('Failed to load cities:', e);
            }
        }
        
        function renderCityList() {
            const search = document.getElementById('city-search').value.toLowerCase();
            const list = document.getElementById('city-list');
            
            const filtered = allCities.filter(c => c.toLowerCase().includes(search));
            
            if (filtered.length === 0) {
                list.innerHTML = '<div class="empty-state">לא נמצאו ערים</div>';
                return;
            }
            
            list.innerHTML = filtered.map(city => `
                <div class="city-item ${selectedCities.includes(city) ? 'selected' : ''}" 
                     onclick="toggleCity('${city.replace(/'/g, "\\'")}')">
                    ${selectedCities.includes(city) ? '✅ ' : ''}${city}
                </div>
            `).join('');
        }
        
        function filterCities() {
            renderCityList();
            document.getElementById('selected-count').textContent = selectedCities.length;
        }
        
        function toggleCity(city) {
            const idx = selectedCities.indexOf(city);
            if (idx > -1) {
                selectedCities.splice(idx, 1);
            } else {
                selectedCities.push(city);
            }
            renderCityList();
            renderSelectedCities();
            document.getElementById('selected-count').textContent = selectedCities.length;
        }
        
        function renderSelectedCities() {
            const container = document.getElementById('selected-cities-list');
            const miniContainer = document.getElementById('selected-cities-mini');
            
            if (selectedCities.length === 0) {
                container.innerHTML = '<div class="empty-state">לא נבחרו ערים</div>';
                miniContainer.innerHTML = '';
                return;
            }
            
            container.innerHTML = selectedCities.map(city => 
                '<span class="chip">' + city + '<span class="chip-remove" onclick="toggleCity(\'' + city.replace(/'/g, "\\'") + '\')">×</span></span>'
            ).join('');
            
            miniContainer.innerHTML = selectedCities.slice(0, 5).map(city => 
                '<span class="city-chip-mini">' + city + '</span>'
            ).join('') + (selectedCities.length > 5 ? '<span class="city-chip-mini">+' + (selectedCities.length - 5) + '</span>' : '');
        }
        
        async function saveCities() {
            try {
                await fetch('/api/led/sequence', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({
                        cities: selectedCities,
                        alertDuration: parseInt(document.getElementById('alert-duration').value),
                        sequences: sequences
                    })
                });
                showToast('✅ הערים נשמרו', 'success');
            } catch (e) {
                showToast('❌ שגיאה בשמירה', 'error');
            }
        }
        
        // LED Sequence
        function setAlertType(type) {
            alertType = type;
            document.querySelectorAll('.alert-type-btn').forEach(btn => {
                btn.classList.toggle('active', btn.getAttribute('onclick').includes(type));
            });
            renderSequenceSteps();
        }
        
        function addStep() {
            sequences[alertType].push({action: 'power_on', value: 1, duration: 1});
            renderSequenceSteps();
        }
        
        function removeStep(idx) {
            sequences[alertType].splice(idx, 1);
            renderSequenceSteps();
        }
        
        function updateStep(idx, field, value) {
            sequences[alertType][idx][field] = value;
        }
        
        function renderSequenceSteps() {
            const container = document.getElementById('sequence-steps');
            
            if (sequences[alertType].length === 0) {
                container.innerHTML = '<div class="empty-state">לחץ על "הוסף פעולה" להוספת שלב</div>';
                return;
            }
            
            const ledColors = [
                {value: '#ff0000', label: '🔴 אדום'},
                {value: '#00ff00', label: '🟢 ירוק'},
                {value: '#0000ff', label: '🔵 כחול'},
                {value: '#ffffff', label: '⚪ לבן'},
                {value: '#ffa500', label: '🟠 כתום'},
                {value: '#00ffff', label: '🔷 ציאן'},
                {value: '#800080', label: '🟣 סגול'},
                {value: '#ffff00', label: '🟡 צהוב'},
                {value: '#00ced1', label: '💠 טורקיז'},
                {value: '#ff69b4', label: '🌸 ורוד'},
                {value: '#ff0000', label: '⚡.flash'},
                {value: '#000000', label: '🌊 fade'},
                {value: 'off', label: '⚫ כיבוי'}
            ];
            
            container.innerHTML = sequences[alertType].map((step, idx) => `
                <div class="sequence-step">
                    <select onchange="updateStep(${idx}, 'action', this.value); renderSequenceSteps();">
                        <option value="power_on" ${step.action === 'power_on' ? 'selected' : ''}>🔴 הדלקה</option>
                        <option value="power_off" ${step.action === 'power_off' ? 'selected' : ''}>⚫ כיבוי</option>
                        <option value="light_color" ${step.action === 'light_color' ? 'selected' : ''}>🎨 צבע</option>
                        <option value="bright_up" ${step.action === 'bright_up' ? 'selected' : ''}>☀️ בהירות+</option>
                        <option value="bright_down" ${step.action === 'bright_down' ? 'selected' : ''}>🌙 בהירות-</option>
                    </select>
                    
                    ${step.action === 'light_color' ? `
                        <div class="color-select-container">
                            <select onchange="updateStep(${idx}, 'color', this.value)">
                                ${ledColors.map(c => `<option value="${c.value}" ${step.color === c.value ? 'selected' : ''}>${c.label}</option>`).join('')}
                            </select>
                        </div>
                    ` : ''}
                    
                    ${step.action === 'bright_up' || step.action === 'bright_down' ? `
                        <div class="slider-container">
                            <input type="range" min="1" max="10" step="1" value="${step.value || 1}"
                                   onchange="updateStep(${idx}, 'value', parseInt(this.value)); this.nextElementSibling.textContent = this.value">
                            <span class="slider-value">${step.value || 1}</span>
                        </div>
                    ` : ''}
                    
                    ${(step.action === 'power_on' || step.action === 'power_off') ? `
                        <div class="slider-container">
                            <input type="range" min="0.5" max="10" step="0.5" value="${step.duration || 1}"
                                   onchange="updateStep(${idx}, 'duration', parseFloat(this.value)); this.nextElementSibling.textContent = this.value + 's'">
                            <span class="slider-value">${step.duration || 1}s</span>
                        </div>
                    ` : ''}
                    
                    ${step.action === 'light_color' ? `
                        <div class="slider-container">
                            <input type="range" min="0.5" max="10" step="0.5" value="${step.duration || 1}"
                                   onchange="updateStep(${idx}, 'duration', parseFloat(this.value)); this.nextElementSibling.textContent = this.value + 's'">
                            <span class="slider-value">${step.duration || 1}s</span>
                        </div>
                    ` : ''}
                    
                    <button class="remove" onclick="removeStep(${idx})">✕ מחק</button>
                </div>
            `).join('');
        }
        
        function applyTemplate(template) {
            if (template === 'blink') {
                sequences[alertType] = [
                    {action: 'power_on', duration: 0.5, color: '#ff0000'},
                    {action: 'light_color', duration: 0.5, color: '#ff0000'},
                    {action: 'power_off', duration: 0.5},
                    {action: 'light_color', duration: 0.5, color: '#000000'}
                ];
            } else if (template === 'pulse') {
                sequences[alertType] = [
                    {action: 'bright_up', duration: 2},
                    {action: 'bright_down', duration: 2}
                ];
            } else if (template === 'warning') {
                sequences[alertType] = [
                    {action: 'power_on', duration: 1, color: '#ff0000'},
                    {action: 'light_color', duration: 1, color: '#ff0000'},
                    {action: 'power_off', duration: 1},
                    {action: 'light_color', duration: 1, color: '#000000'}
                ];
            }
            renderSequenceSteps();
        }
        
        async function saveSequence() {
            try {
                const res = await fetch('/api/led/sequence', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({
                        cities: selectedCities,
                        alertDuration: parseInt(document.getElementById('alert-duration').value),
                        sequences: sequences
                    })
                });
                const data = await res.json();
                if (data.success) {
                    showToast('✅ הרצף נשמר', 'success');
                }
            } catch (e) {
                showToast('❌ שגיאה בשמירה', 'error');
            }
        }
        
        async function saveDuration() {
            const duration = parseInt(document.getElementById('alert-duration').value);
            try {
                await fetch('/api/led/duration', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({duration})
                });
            } catch (e) {}
        }
        
        // IR
        const irCommands = [
            {id: 'power_on', name: '🔴 הדלקה'},
            {id: 'power_off', name: '⚫ כיבוי'},
            {id: 'bright_up', name: '☀️ בהירות+'},
            {id: 'bright_down', name: '🌙 בהירות-'},
            {id: 'red', name: '🔴 אדום'},
            {id: 'green', name: '🟢 ירוק'},
            {id: 'blue', name: '🔵 כחול'},
            {id: 'white', name: '⚪ לבן'}
        ];
        
        function renderIRCommands() {
            const container = document.getElementById('ir-commands');
            container.innerHTML = irCommands.map(cmd => `
                <div class="sequence-step">
                    <span style="flex:1">${cmd.name}</span>
                    <button class="btn btn-secondary btn-small" onclick="recordIR('${cmd.id}')">🎙️ הקלט</button>
                    <span class="status-indicator">
                        <span class="status-dot red"></span>
                    </span>
                </div>
            `).join('');
        }
        
        async function recordIR(commandId) {
            document.getElementById('record-modal').classList.add('show');
            try {
                const res = await fetch('/api/ir/record', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify({command: commandId})
                });
                const data = await res.json();
                showToast(data.calibrated ? '✅ נשמר' : '⚠️ לא נשמר', 
                    data.calibrated ? 'success' : 'warning');
            } catch (e) {
                showToast('❌ שגיאה', 'error');
            }
            document.getElementById('record-modal').classList.remove('show');
            renderIRCommands();
        }
        
        // System
        async function refreshLogs() {
            try {
                const res = await fetch('/api/system/logs');
                const data = await res.json();
                document.getElementById('logs').textContent = data.logs || 'אין לוגים';
            } catch (e) {
                document.getElementById('logs').textContent = 'שגיאה בטעינת לוגים';
            }
        }
        
        async function restart() {
            if (!confirm('לאתחל את המערכת?')) return;
            try {
                await fetch('/api/system/restart', {method: 'POST'});
                showToast('🔄 מאתחל...', 'warning');
            } catch (e) {}
        }
        
        async function checkUpdate() {
            showToast('🔄 בודק עדכונים...', 'warning');
            try {
                const res = await fetch('/api/system/check_update', {method: 'POST'});
                const data = await res.json();
                showToast(data.message || '✅ הושלם', data.success ? 'success' : 'warning');
            } catch (e) {
                showToast('❌ שגיאה', 'error');
            }
        }
        
        async function testLights() {
            try {
                await fetch('/api/system/test_lights', {method: 'POST'});
                showToast('💡 בודק אורות...', 'success');
            } catch (e) {
                showToast('❌ שגיאה', 'error');
            }
        }
        
        // Init
        initDarkMode();
        renderSelectedCities();
        renderSequenceSteps();
        renderIRCommands();
        
        // Check WiFi status
        fetch('/api/wifi/status').then(r => r.json()).then(data => {
            if (data.connected) updateWiFiStatus(true, data.ssid);
        });
        
        // Load initial cities
        loadCities();
    </script>
</body>
</html>
)rawliteral";
