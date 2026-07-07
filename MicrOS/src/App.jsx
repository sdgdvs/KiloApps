import { useState, useRef, useEffect, useCallback } from 'react';
import { DEFAULT_VFS } from './defaultVfs';
import './App.css';

const MICROS_VERSION = '0.3.0';

const APPS = [
  { id: 'microexplorer', title: 'File Browser', url: '/apps/microexplorer.html', icon: '/assets/icons/microexplorer.ico', w: 400, h: 300 },
  { id: 'microchat', title: 'MicroChat', url: '/apps/microchat.html', exeUrl: '/exe/MicroChat.exe', icon: '/assets/icons/microchat.ico', w: 400, h: 300 },
  { id: 'microchatserver', title: 'MicroChat Server', url: '/apps/microchatserver.html', exeUrl: '/exe/MicroChatServer.exe', icon: '/assets/icons/microchatserver.ico', w: 350, h: 250 },
  { id: 'micropad', title: 'MicroPad', url: '/apps/micropad.html', exeUrl: '/exe/MicroPad.exe', icon: '/assets/icons/micropad.ico', w: 500, h: 400 },
  { id: 'microcalc', title: 'MicroCalc', url: '/apps/microcalc.html', exeUrl: '/exe/MicroCalc.exe', icon: '/assets/icons/microcalc.ico', w: 320, h: 340 },
  { id: 'micropaint', title: 'MicroPaint', url: '/apps/micropaint.html', exeUrl: '/exe/MicroPaint.exe', icon: '/assets/icons/micropaint.ico', w: 600, h: 450 },
  { id: 'micromines', title: 'MicroMines', url: '/apps/micromines.html', exeUrl: '/exe/MicroMines.exe', icon: '/assets/icons/micromines.ico', w: 250, h: 290 },
  { id: 'microclock', title: 'MicroClock', url: '/apps/microclock.html', exeUrl: '/exe/MicroClock.exe', icon: '/assets/icons/microclock.ico', w: 220, h: 140 },
  { id: 'microtask', title: 'MicroTask', url: '/apps/microtask.html', exeUrl: '/exe/MicroTask.exe', icon: '/assets/icons/microtask.ico', w: 400, h: 300 },
  { id: 'microbbs', title: 'MicroBBS', url: '/apps/microbbs.html', exeUrl: '/exe/MicroBBS.exe', icon: '/assets/icons/microbbs.ico', w: 720, h: 480 }
];

function Window({ app, onClose, onFocus, onMinimize, vfs, setVfs, requestVfsModal, openApps, closeApp }) {
  const [pos, setPos] = useState({ x: app.x, y: app.y });
  const [dragging, setDragging] = useState(false);
  const offset = useRef({ x: 0, y: 0 });
  const iframeRef = useRef(null);

  const handlePointerDown = (e) => {
    onFocus();
    setDragging(true);
    offset.current = { x: e.clientX - pos.x, y: e.clientY - pos.y };
    e.target.setPointerCapture(e.pointerId);
  };

  const handlePointerMove = (e) => {
    if (dragging) {
      setPos({ x: e.clientX - offset.current.x, y: e.clientY - offset.current.y });
    }
  };

  const handlePointerUp = (e) => {
    setDragging(false);
    e.target.releasePointerCapture(e.pointerId);
  };

  // Handle postMessage from iframe
  useEffect(() => {
    const handleMessage = (e) => {
      if (e.source !== iframeRef.current?.contentWindow) return;
      const { type, path, data, requestId } = e.data;

      if (type === 'VFS_READ') {
        const fileData = vfs[path] || '';
        e.source.postMessage({ type: 'VFS_READ_DONE', requestId, data: fileData, path }, '*');
      } 
      else if (type === 'VFS_WRITE') {
        setVfs(prev => ({ ...prev, [path]: data }));
        e.source.postMessage({ type: 'VFS_WRITE_DONE', requestId, path }, '*');
      } 
      else if (type === 'VFS_LIST') {
        e.source.postMessage({ type: 'VFS_LIST_DONE', requestId, files: Object.keys(vfs) }, '*');
      }
      else if (type === 'VFS_SHOW_OPEN') {
        requestVfsModal('open', (selectedPath) => {
          e.source.postMessage({ type: 'VFS_SHOW_OPEN_DONE', requestId, path: selectedPath }, '*');
        });
      }
      else if (type === 'VFS_SHOW_SAVE') {
        requestVfsModal('save', (selectedPath) => {
          e.source.postMessage({ type: 'VFS_SHOW_SAVE_DONE', requestId, path: selectedPath }, '*');
        });
      }
      else if (type === 'OS_LAUNCH_APP') {
        // e.g. File Explorer launching an app
        // Dispatched as a custom event so App can catch it
        window.dispatchEvent(new CustomEvent('os-launch-app', { detail: e.data }));
      }
      else if (type === 'OS_LIST_TASKS') {
        const tasks = openApps.map(a => ({ id: a.instanceId, title: a.title }));
        e.source.postMessage({ type: 'OS_LIST_TASKS_DONE', requestId, tasks }, '*');
      }
      else if (type === 'OS_KILL_TASK') {
        closeApp(e.data.taskId);
      }
    };
    window.addEventListener('message', handleMessage);
    return () => window.removeEventListener('message', handleMessage);
  }, [vfs, setVfs, requestVfsModal, openApps, closeApp]);

  return (
    <div 
      className="xp-window" 
      style={{
        left: pos.x, top: pos.y, width: app.w, height: app.h, zIndex: app.zIndex,
        display: app.minimized ? 'none' : 'flex'
      }}
      onPointerDownCapture={onFocus}
    >
      <div 
        className="xp-titlebar"
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerUp}
      >
        <div className="xp-title" style={{display:'flex', alignItems:'center', gap:'6px'}}>
          {app.icon && <img src={app.icon} alt="" style={{width:'16px', height:'16px', imageRendering:'pixelated'}} />}
          {app.title}
        </div>
        <div className="xp-controls">
          {app.exeUrl && (
            <a href={app.exeUrl} download className="xp-btn" title="Download Native .exe" onClick={e => e.stopPropagation()} style={{textDecoration:'none', color:'white', background:'linear-gradient(to bottom, #77a2df, #3b73c4)', marginRight:'5px'}}>↓</a>
          )}
          <div className="xp-btn" onClick={(e) => { e.stopPropagation(); onMinimize(); }}>_</div>
          <div className="xp-btn xp-btn-close" onClick={(e) => { e.stopPropagation(); onClose(); }}>X</div>
        </div>
      </div>
      <div className="xp-content">
        {/* Transparent overlay while dragging to prevent iframe from stealing mouse */}
        {dragging && <div style={{position:'absolute', top:0, left:0, right:0, bottom:0, zIndex:10}} />}
        <iframe ref={iframeRef} className="xp-iframe" src={app.url} title={app.title} sandbox="allow-scripts allow-same-origin" />
      </div>
    </div>
  );
}

function App() {
  const [screen, setScreen] = useState('login'); // 'login' | 'os'
  const [vfs, setVfs] = useState(DEFAULT_VFS);
  const [openApps, setOpenApps] = useState([]);
  const [zIndexCounter, setZIndexCounter] = useState(10);
  const [time, setTime] = useState("");
  const [startOpen, setStartOpen] = useState(false);
  const [activeAppId, setActiveAppId] = useState(null);
  
  // OS Modals
  const [modal, setModal] = useState(null); // { type: 'shutdown' | 'vfs_open' | 'vfs_save', callback: fn }
  const [modalInput, setModalInput] = useState('');

  // Clock
  useEffect(() => {
    const timer = setInterval(() => {
      setTime(new Date().toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'}));
    }, 1000);
    return () => clearInterval(timer);
  }, []);

  // Listen for File Explorer launching apps
  useEffect(() => {
    const handler = (e) => {
      const { appId, path } = e.detail;
      const appDef = APPS.find(a => a.id === appId);
      if (appDef) {
        // Simple way to pass path to a newly launched app is via URL hash
        openApp({ ...appDef, url: `${appDef.url}#${path}` });
      }
    };
    window.addEventListener('os-launch-app', handler);
    return () => window.removeEventListener('os-launch-app', handler);
  }, [openApps]);

  const openApp = (appDef) => {
    setStartOpen(false);
    const existing = openApps.find(a => a.id === appDef.id && a.url === appDef.url);
    if (existing) {
      focusApp(existing.instanceId);
      if (existing.minimized) toggleMinimize(existing.instanceId);
      return;
    }
    const newApp = { 
      ...appDef, 
      instanceId: Math.random().toString(), 
      x: 50 + ((openApps.length % 10) * 20), 
      y: 50 + ((openApps.length % 10) * 20),
      zIndex: zIndexCounter + 1,
      minimized: false
    };
    setZIndexCounter(z => z + 1);
    setOpenApps([...openApps, newApp]);
    setActiveAppId(newApp.instanceId);
  };

  const closeApp = (instanceId) => {
    setOpenApps(prev => prev.filter(a => a.instanceId !== instanceId));
  };

  const focusApp = (instanceId) => {
    setZIndexCounter(z => z + 1);
    setOpenApps(apps => apps.map(a => a.instanceId === instanceId ? { ...a, zIndex: zIndexCounter + 1 } : a));
    setActiveAppId(instanceId);
  };

  const toggleMinimize = (instanceId) => {
    setStartOpen(false);
    setOpenApps(apps => apps.map(a => a.instanceId === instanceId ? { ...a, minimized: !a.minimized } : a));
    focusApp(instanceId);
  };

  const handleUploadState = () => {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.onchange = (e) => {
      const file = e.target.files[0];
      const reader = new FileReader();
      reader.onload = (ev) => {
        try {
          const parsed = JSON.parse(ev.target.result);
          setVfs(parsed);
          setScreen('os');
        } catch (err) {
          alert("Invalid state file");
        }
      };
      reader.readAsText(file);
    };
    input.click();
  };

  const doShutdown = () => {
    const filename = modalInput || 'micros_state.json';
    const blob = new Blob([JSON.stringify(vfs, null, 2)], { type: "application/json" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename.endsWith('.json') ? filename : filename + '.json';
    a.click();
    URL.revokeObjectURL(url);
    
    setModal(null);
    setOpenApps([]);
    setVfs(DEFAULT_VFS);
    setScreen('login');
  };

  const requestVfsModal = useCallback((type, callback) => {
    setModal({ type: `vfs_${type}`, callback });
    setModalInput('');
  }, []);

  if (screen === 'login') {
    return (
      <div className="login-screen">
        <div className="login-title">MicrOS</div>
        <div style={{color:'white', marginBottom: '20px', fontSize: '14px'}}>Version {MICROS_VERSION}</div>
        <div className="login-box">
          <button className="login-btn" onClick={() => setScreen('os')}>Start Fresh</button>
          <button className="login-btn" onClick={handleUploadState}>Upload Saved State (.json)</button>
        </div>
      </div>
    );
  }

  return (
    <>
      <div className="desktop" onClick={() => setStartOpen(false)}>
        {APPS.map(app => (
          <div key={app.id} className="desktop-icon" onDoubleClick={() => openApp(app)}>
            <img src={app.icon} alt={app.title} style={{width:'32px', height:'32px', imageRendering: 'pixelated'}} />
            <div className="icon-label">{app.title}</div>
          </div>
        ))}
        {openApps.map(app => (
          <Window 
            key={app.instanceId} 
            app={app} 
            onClose={() => closeApp(app.instanceId)}
            onFocus={() => focusApp(app.instanceId)}
            onMinimize={() => toggleMinimize(app.instanceId)}
            vfs={vfs}
            setVfs={setVfs}
            requestVfsModal={requestVfsModal}
            openApps={openApps}
            closeApp={closeApp}
          />
        ))}
      </div>
      
      {startOpen && (
        <div className="start-menu">
          <div className="start-header">MicrOS User</div>
          <div className="start-items">
            {APPS.map(app => (
              <div key={app.id} className="start-item" onClick={() => openApp(app)} style={{display: 'flex', alignItems: 'center', gap: '8px'}}>
                <img src={app.icon} alt={app.title} style={{width:'24px', height:'24px', imageRendering: 'pixelated'}} />
                {app.title}
              </div>
            ))}
            <div className="start-item" onClick={() => { setStartOpen(false); alert(`MicrOS Version ${MICROS_VERSION}`); }}>
              About MicrOS
            </div>
          </div>
          <div className="start-footer">
            <a href="/exe/MicroApps.zip" download className="shutdown-btn" style={{textDecoration:'none', textAlign:'center', display:'block', marginBottom:'5px', background:'linear-gradient(to bottom, #77a2df, #3b73c4)'}}>
              Download All .exe (.zip)
            </a>
            <button className="shutdown-btn" onClick={() => { setStartOpen(false); setModal({ type: 'shutdown' }); setModalInput('micros_state.json'); }}>
              Shutdown...
            </button>
          </div>
        </div>
      )}

      <div className="taskbar">
        <div className={`start-button ${startOpen ? 'open' : ''}`} onClick={() => setStartOpen(!startOpen)}>start</div>
        <div className="taskbar-items">
          {openApps.map(app => {
            const baseApp = APPS.find(a => a.id === app.id);
            return (
              <div key={app.instanceId} className={`taskbar-item ${(!app.minimized && app.zIndex === Math.max(...openApps.map(a => a.zIndex))) ? 'active' : ''}`} onClick={() => toggleMinimize(app.instanceId)} style={{display: 'flex', alignItems: 'center', gap: '4px'}}>
                {baseApp && <img src={baseApp.icon} alt="" style={{width:'16px', height:'16px', imageRendering: 'pixelated'}} />}
                {app.title}
              </div>
            );
          })}
        </div>
        <div className="clock-tray">{time}</div>
      </div>

      {modal && (
        <div className="os-modal-overlay">
          <div className="os-modal">
            <div className="os-modal-title">
              {modal.type === 'shutdown' ? 'Shutdown MicrOS' : modal.type === 'vfs_open' ? 'Open Virtual File' : 'Save Virtual File'}
            </div>
            <div className="os-modal-content">
              {modal.type === 'shutdown' && (
                <>
                  <p>Save your virtual filesystem state before shutting down:</p>
                  <input value={modalInput} onChange={e => setModalInput(e.target.value)} placeholder="micros_state.json" />
                </>
              )}
              {modal.type === 'vfs_save' && (
                <>
                  <p>Enter filename to save:</p>
                  <input value={modalInput} onChange={e => setModalInput(e.target.value)} placeholder="/file.txt" />
                </>
              )}
              {modal.type === 'vfs_open' && (
                <>
                  <p>Select a file to open:</p>
                  <select size="5" style={{width:'100%'}} onChange={e => setModalInput(e.target.value)}>
                    {Object.keys(vfs).map(k => <option key={k} value={k}>{k}</option>)}
                  </select>
                </>
              )}
              <div className="os-modal-buttons">
                <button onClick={() => {
                  if (modal.type === 'shutdown') doShutdown();
                  else { modal.callback(modalInput); setModal(null); }
                }}>OK</button>
                <button onClick={() => { 
                  if (modal.callback) modal.callback(null); 
                  setModal(null); 
                }}>Cancel</button>
              </div>
            </div>
          </div>
        </div>
      )}
    </>
  );
}

export default App;
