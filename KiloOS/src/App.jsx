import { useState, useRef, useEffect, useCallback } from 'react';
import { DEFAULT_VFS } from './defaultVfs';
import './App.css';

const MICROS_VERSION = '0.3.5';

const FOLDER_ICON = "data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='%23ffd700'><path d='M10 4H4c-1.1 0-1.99.9-1.99 2L2 18c0 1.1.9 2 2 2h16c1.1 0 2-.9 2-2V8c0-1.1-.9-2-2-2h-8l-2-2z'/></svg>";

const FOLDERS = [
  { id: 'System', title: 'System Tools', icon: FOLDER_ICON, isFolder: true, w: 450, h: 350 },
  { id: 'Media', title: 'Media & Arts', icon: FOLDER_ICON, isFolder: true, w: 450, h: 350 },
  { id: 'Office', title: 'Office', icon: FOLDER_ICON, isFolder: true, w: 450, h: 350 },
  { id: 'Games', title: 'Games', icon: FOLDER_ICON, isFolder: true, w: 450, h: 350 },
  { id: 'Network', title: 'Network', icon: FOLDER_ICON, isFolder: true, w: 450, h: 350 },
  { id: 'Dev', title: 'Development', icon: FOLDER_ICON, isFolder: true, w: 450, h: 350 }
];

const APPS = [
  { id: 'kexplorer', title: 'File Browser', url: '/apps/kexplorer.html', icon: '/assets/icons/kexplorer.ico', w: 400, h: 300, folder: 'System' },
  { id: 'kchat', title: 'KChat', url: '/apps/kchat.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kchat.ico', w: 400, h: 300, folder: 'Network' },
  { id: 'kchatserver', title: 'KChat Server', url: '/apps/kchatserver.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kchatserver.ico', w: 350, h: 250, folder: 'Network' },
  { id: 'kpad', title: 'KPad', url: '/apps/kpad.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kpad.ico', w: 500, h: 400, folder: 'Office' },
  { id: 'kcalc', title: 'KCalc', url: '/apps/kcalc.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kcalc.ico', w: 320, h: 340, folder: 'System' },
  { id: 'kpaint', title: 'KPaint', url: '/apps/kpaint.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kpaint.ico', w: 600, h: 450, folder: 'Media' },
  { id: 'kmines', title: 'KMines', url: '/apps/kmines.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kmines.ico', w: 250, h: 290, folder: 'Games' },
  { id: 'kclock', title: 'KClock', url: '/apps/kclock.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kclock.ico', w: 220, h: 140, folder: 'System' },
  { id: 'ktask', title: 'KTask', url: '/apps/ktask.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ktask.ico', w: 400, h: 300, folder: 'System' },
  { id: 'kbbs', title: 'KBBS', url: '/apps/kbbs.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kbbs.ico', w: 720, h: 480, folder: 'Network' },
  { id: 'krogue', title: 'KRogue', url: '/apps/krogue.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/krogue.ico', w: 400, h: 250, folder: 'Games' },
  { id: 'ksnake', title: 'KSnake', url: '/apps/ksnake.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ksnake.ico', w: 320, h: 360, folder: 'Games' },
  { id: 'ktetris', title: 'KTetris', url: '/apps/ktetris.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ktetris.ico', w: 220, h: 440, folder: 'Games' },
  { id: 'kpong', title: 'KPong', url: '/apps/kpong.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kpong.ico', w: 420, h: 340, folder: 'Games' },
  { id: 'kterm', title: 'KTerm', url: '/apps/kterm.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kterm.ico', w: 500, h: 350, folder: 'System' },
  { id: 'kmaze', title: 'KMaze', url: '/apps/kmaze.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kmaze.ico', w: 340, h: 290, folder: 'Games' },
  { id: 'kaudio', title: 'KAudio', url: '/apps/kaudio.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kaudio.ico', w: 420, h: 260, folder: 'Media' },
  { id: 'kwrite', title: 'KWrite', url: '/apps/kwrite.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kwrite.ico', w: 500, h: 400, folder: 'Office' },
  { id: 'kcalendar', title: 'KCalendar', url: '/apps/kcalendar.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kcalendar.ico', w: 320, h: 320, folder: 'Office' },
  { id: 'kdraw', title: 'KDraw', url: '/apps/kdraw.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kdraw.ico', w: 500, h: 400, folder: 'Media' },
  { id: 'ksolitaire', title: 'KSolitaire', url: '/apps/ksolitaire.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ksolitaire.ico', w: 420, h: 450, folder: 'Games' },
  { id: 'kspace', title: 'KSpace', url: '/apps/kspace.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kspace.ico', w: 340, h: 520, folder: 'Games' },
  { id: 'kpac', title: 'KPac', url: '/apps/kpac.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kpac.ico', w: 340, h: 360, folder: 'Games' },
  { id: 'kmail', title: 'KMail', url: '/apps/kmail.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kmail.ico', w: 500, h: 400, folder: 'Network' },
  { id: 'kmedia', title: 'KMedia', url: '/apps/kmedia.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kmedia.ico', w: 320, h: 150, folder: 'Media' },
  { id: 'kimage', title: 'KImage', url: '/apps/kimage.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kimage.ico', w: 500, h: 500, folder: 'Media' },
  { id: 'knet', title: 'KNet', url: '/apps/knet.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/knet.ico', w: 600, h: 500, folder: 'Network' },
  { id: 'kdb', title: 'KDB', url: '/apps/kdb.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kdb.ico', w: 500, h: 400, folder: 'Dev' },
  { id: 'kscript', title: 'KScript', url: '/apps/kscript.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kscript.ico', w: 400, h: 300, folder: 'Dev' },
  { id: 'kchess', title: 'KChess', url: '/apps/kchess.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kchess.ico', w: 480, h: 500, folder: 'Games' },
  { id: 'ktype', title: 'KType', url: '/apps/ktype.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ktype.ico', w: 400, h: 500, folder: 'Dev' },
  { id: 'kchart', title: 'KChart', url: '/apps/kchart.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kchart.ico', w: 400, h: 300, folder: 'Media' },
  { id: 'kzip', title: 'KZip', url: '/apps/kzip.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kzip.ico', w: 400, h: 300, folder: 'System' },
  { id: 'knote', title: 'KNote', url: '/apps/knote.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/knote.ico', w: 250, h: 250, folder: 'Office' },
  { id: 'kcolor', title: 'KColor', url: '/apps/kcolor.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kcolor.ico', w: 300, h: 200, folder: 'Media' },
  { id: 'ksound', title: 'KSound', url: '/apps/ksound.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ksound.ico', w: 400, h: 200, folder: 'Media' },
  { id: 'kpass', title: 'KPass', url: '/apps/kpass.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kpass.ico', w: 300, h: 150, folder: 'System' },
  { id: 'kping', title: 'KPing', url: '/apps/kping.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kping.ico', w: 400, h: 300, folder: 'System' },
  { id: 'khex', title: 'KHex', url: '/apps/khex.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/khex.ico', w: 300, h: 200, folder: 'System' },
  { id: 'ksys', title: 'KSys', url: '/apps/ksys.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ksys.ico', w: 400, h: 300, folder: 'System' },
  { id: 'kmandel', title: 'KMandel', url: '/apps/kmandel.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kmandel.ico', w: 400, h: 400, folder: 'Media' },
  { id: 'ktimer', title: 'KTimer', url: '/apps/ktimer.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ktimer.ico', w: 300, h: 200, folder: 'System' },
  { id: 'ksynth', title: 'KSynth', url: '/apps/ksynth.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ksynth.ico', w: 300, h: 200, folder: 'Media' },
  { id: 'kfont', title: 'KFont', url: '/apps/kfont.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kfont.ico', w: 400, h: 300, folder: 'System' },
  { id: 'kconverter', title: 'KConverter', url: '/apps/kconverter.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kconverter.ico', w: 350, h: 400, folder: 'System' },
  { id: 'ktodo', title: 'KTodo', url: '/apps/ktodo.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/ktask.ico', w: 350, h: 450, folder: 'System' },
  { id: 'kgraph', title: 'KGraph', url: '/apps/kgraph.html', exeUrl: '/exe/KApps.zip', icon: '/assets/icons/kchart.ico', w: 500, h: 400, folder: 'System' },
  { id: 'kquarantine', title: 'Q̷u̷a̷r̷a̷n̷t̷i̷n̷e̷', url: '#', exeUrl: null, icon: '/assets/icons/ksys.ico', w: 300, h: 200 }
];

function Window({ app, onClose, onFocus, onMinimize, vfs, setVfs, requestVfsModal, openApps, closeApp }) {
  const [pos, setPos] = useState({ x: app.x, y: app.y });
  const [dragging, setDragging] = useState(false);
  const [isMaximized, setIsMaximized] = useState(false);
  const offset = useRef({ x: 0, y: 0 });
  const iframeRef = useRef(null);
  const isActive = !app.minimized && app.zIndex === Math.max(...openApps.map(a => a.zIndex));

  const toggleMaximize = () => {
    setIsMaximized(!isMaximized);
    onFocus();
  };

  const handlePointerDown = (e) => {
    onFocus();
    if (isMaximized) return;
    setDragging(true);
    offset.current = { x: e.clientX - pos.x, y: e.clientY - pos.y };
    e.target.setPointerCapture(e.pointerId);
  };

  const handlePointerMove = (e) => {
    if (dragging) {
      let newY = Math.max(0, e.clientY - offset.current.y);
      setPos({ x: e.clientX - offset.current.x, y: newY });
    }
  };

  const handlePointerUp = (e) => {
    setDragging(false);
    e.target.releasePointerCapture(e.pointerId);
  };

  // Ensure windows stay reachable even if viewport resizes or they spawn out of bounds
  useEffect(() => {
    const handleResize = () => {
      setPos(p => {
        let newX = p.x;
        let newY = p.y;
        if (newY < 0) newY = 0;
        if (newY > window.innerHeight - 60) newY = Math.max(0, window.innerHeight - 60);
        if (newX > window.innerWidth - 60) newX = window.innerWidth - 60;
        if (newX < -app.w + 60) newX = -app.w + 60;
        
        if (newX !== p.x || newY !== p.y) return { x: newX, y: newY };
        return p;
      });
    };
    handleResize();
    window.addEventListener('resize', handleResize);
    return () => window.removeEventListener('resize', handleResize);
  }, [app.w]);

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
      className={`xp-window ${isActive ? 'active' : ''} ${isMaximized ? 'maximized' : ''}`} 
      style={{
        left: isMaximized ? 0 : pos.x, 
        top: isMaximized ? 0 : pos.y, 
        width: isMaximized ? '100%' : app.w, 
        height: isMaximized ? 'calc(100% - 40px)' : app.h, 
        zIndex: app.zIndex,
        display: app.minimized ? 'none' : 'flex'
      }}
      onPointerDownCapture={onFocus}
    >
      <div 
        className="xp-titlebar"
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerUp}
        onDoubleClick={toggleMaximize}
      >
        <div className="xp-title" style={{display:'flex', alignItems:'center', gap:'6px'}}>
          {app.icon && <img src={app.icon} alt="" style={{width:'16px', height:'16px', imageRendering:'pixelated'}} />}
          {app.title}
        </div>
        <div className="xp-controls">
          {app.exeUrl && (
            <a href={app.exeUrl} download className="xp-btn" title="Download KApps.zip" onClick={e => e.stopPropagation()} style={{textDecoration:'none', color:'white', background:'linear-gradient(to bottom, #77a2df, #3b73c4)', marginRight:'5px'}}>↓</a>
          )}
          <div className="xp-btn" onClick={(e) => { e.stopPropagation(); onMinimize(); }}>_</div>
          <div className="xp-btn" onClick={(e) => { e.stopPropagation(); toggleMaximize(); }}>☐</div>
          <div className="xp-btn xp-btn-close" onClick={(e) => { e.stopPropagation(); onClose(); }}>X</div>
        </div>
      </div>
      <div className="xp-content">
        {/* Transparent overlay while dragging to prevent iframe from stealing mouse */}
        {dragging && <div style={{position:'absolute', top:0, left:0, right:0, bottom:0, zIndex:10}} />}
        {app.isFolder ? (
          <div className="folder-content" style={{ display: 'flex', flexWrap: 'wrap', gap: '15px', padding: '15px', alignContent: 'flex-start', overflowY: 'auto', width: '100%', height: '100%', background: 'var(--bg-color)' }}>
            {APPS.filter(a => a.folder === app.id).map(child => (
              <div 
                key={child.id} 
                className="desktop-icon" 
                style={{ position: 'relative', display: 'flex', flexDirection: 'column', alignItems: 'center' }}
                onClick={(e) => { e.stopPropagation(); }}
                onDoubleClick={(e) => { e.stopPropagation(); window.dispatchEvent(new CustomEvent('os-launch-app', { detail: { appId: child.id, path: '' } })); }}
              >
                <img src={child.icon} alt={child.title} style={{width:'32px', height:'32px', imageRendering: 'pixelated'}} />
                <div className="icon-label" style={{color: 'var(--text-main)', textAlign: 'center', marginTop: '4px'}}>{child.title}</div>
              </div>
            ))}
          </div>
        ) : (
          <iframe 
            ref={iframeRef} 
            className="xp-iframe" 
            src={app.url} 
            title={app.title} 
            sandbox="allow-scripts allow-same-origin allow-downloads allow-popups"
            style={{ opacity: 0, transition: 'opacity 0.3s ease, transform 0.3s cubic-bezier(0.2, 0.8, 0.2, 1)', transform: 'scale(0.98)' }}
            onLoad={(e) => {
              e.target.style.opacity = 1;
              e.target.style.transform = 'scale(1)';
            }}
          />
        )}
      </div>
    </div>
  );
}

function App() {
  const [screen, setScreen] = useState('login'); // 'login' | 'os'

  const playAnomalyAudio = useCallback(() => {
    if (Math.random() < 0.01) {
      const audio = new Audio("data:audio/wav;base64,UklGRigAAABXQVZFZm10IBAAAAABAAEARKwAAIhYAQACABAAZGF0YQQAAAAAAwADAA==");
      audio.volume = 0.5;
      audio.play().catch(() => {});
    }
  }, []);
  const argClicks = useRef(0);
  const handlePixelClick = (e) => {
    e.stopPropagation();
    argClicks.current += 1;
    if (argClicks.current === 5) {
      alert(atob("WyBWLiBdIFlvdSBmb3VuZCBpdC4gVGhlIGFyY2hpdmUgaXMgd2FpdGluZy4="));
    }
  };
  const argAppHistory = useRef([]);
  const [isGlitching, setIsGlitching] = useState(false);
  const [safeMode, setSafeMode] = useState(false);
  const [vfs, setVfs] = useState(DEFAULT_VFS);
  const [openApps, setOpenApps] = useState([]);
  const [zIndexCounter, setZIndexCounter] = useState(10);
  const [time, setTime] = useState("");
  const [startOpen, setStartOpen] = useState(false);
  const [activeAppId, setActiveAppId] = useState(null);
  
  // OS Modals
  const [modal, setModal] = useState(null); // { type: 'shutdown' | 'vfs_open' | 'vfs_save', callback: fn }
  const [modalInput, setModalInput] = useState('');

  // Desktop State
  const [selectedIcon, setSelectedIcon] = useState(null);
  const [contextMenu, setContextMenu] = useState(null);

  const handleDesktopClick = () => {
    setStartOpen(false);
    setSelectedIcon(null);
    setContextMenu(null);
  };

  const handleContextMenu = (e) => {
    e.preventDefault();
    setContextMenu({ x: e.clientX, y: e.clientY });
  };

  // Clock
  useEffect(() => {
    const timer = setInterval(() => {
      const now = new Date();
      if (now.getMinutes() === 33) {
        setTime("S.O.S. REBOOT");
      } else {
        setTime(now.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'}));
      }
    }, 1000);
    return () => clearInterval(timer);
  }, []);

  // Boot sequence
  useEffect(() => {
    if (screen === 'boot') {
      const timer = setTimeout(() => {
        setScreen('os');
      }, 1500);
      return () => clearTimeout(timer);
    }
  }, [screen]);

  // ARG Arc 3 Phase 5: The Architect's Retaliation
  useEffect(() => {
    if (!safeMode) return;
    const retaliationTimer = setInterval(() => {
      setOpenApps(apps => {
        if (apps.length > 0) {
          alert("YOU CANNOT UNPLUG ME.");
          const indexToKill = Math.floor(Math.random() * apps.length);
          return apps.filter((_, i) => i !== indexToKill);
        }
        return apps;
      });
    }, 15000); // Retaliate every 15 seconds if Safe Mode is active
    return () => clearInterval(retaliationTimer);
  }, [safeMode]);

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
    if (appDef.id === 'kquarantine') {
      const pwd = prompt("FATAL EXCEPTION: SECTOR LOCKED.\nREQUIRES LEVEL 9 CLEARANCE PASSPHRASE:");
      if (pwd !== null) {
        setVfs(prev => ({
          ...prev, 
          '/.sys_quarantine_log': (prev['/.sys_quarantine_log'] || '') + `[DENIED] Attempted Passphrase: ${pwd}\n`
        }));
        alert("ACCESS DENIED. INCIDENT LOGGED.");
      }
      return;
    }

    argAppHistory.current.push(appDef.id);
    if (argAppHistory.current.length > 3) argAppHistory.current.shift();
    if (argAppHistory.current.join(',') === 'kclock,kcalc,kterm') {
      setIsGlitching(true);
      setTimeout(() => setIsGlitching(false), 300);
      console.log("%c[SYSTEM] Visual anomaly detected.", "color: red; font-size: 8px;");
    }
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
          setScreen('boot');
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
        <div className="login-box">
          <div className="login-avatar">
            <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"></path><circle cx="12" cy="7" r="4"></circle></svg>
          </div>
          <div className="login-title">KiloOS</div>
          <div style={{color:'var(--text-muted)', marginBottom: '20px', fontSize: '13px'}}>Version {MICROS_VERSION}</div>
          <button className="login-btn" onClick={() => setScreen('boot')} style={{width: '100%', justifyContent: 'center'}}>Start Fresh</button>
          <button className="login-btn" onClick={handleUploadState} style={{width: '100%', justifyContent: 'center', background: 'rgba(255,255,255,0.05)', color: 'var(--text-main)'}}>Upload Saved State</button>
        </div>
      </div>
    );
  }

  if (screen === 'boot') {
    return (
      <div className="boot-screen">
        <div className="login-title" style={{marginBottom: '20px'}}>KiloOS</div>
        <div className="boot-progress-container">
          <div className="boot-progress-bar"></div>
        </div>
      </div>
    );
  }

  return (
    <div className={`os-wrapper ${isGlitching ? 'arg-corrupted-glitch' : ''} ${safeMode ? 'safe-mode-theme' : ''}`} onContextMenu={handleContextMenu} onPointerDownCapture={playAnomalyAudio}>
      <div className="desktop" onClick={handleDesktopClick}>
        <div 
          onClick={handlePixelClick}
          style={{ position: 'absolute', bottom: '0px', right: '0px', width: '5px', height: '5px', opacity: 0.01, zIndex: 9999, cursor: 'default' }}
          title=" "
        />
        {FOLDERS.map(folder => (
          <div 
            key={folder.id} 
            className={`desktop-icon ${selectedIcon === folder.id ? 'selected' : ''}`} 
            onClick={(e) => { e.stopPropagation(); setSelectedIcon(folder.id); setContextMenu(null); setStartOpen(false); }}
            onDoubleClick={() => { setSelectedIcon(null); openApp(folder); }}
          >
            <img src={folder.icon} alt={folder.title} style={{width:'32px', height:'32px', imageRendering: 'pixelated'}} />
            <div className="icon-label">{folder.title}</div>
          </div>
        ))}
        {APPS.filter(a => !a.folder).map(app => (
          <div 
            key={app.id} 
            className={`desktop-icon ${selectedIcon === app.id ? 'selected' : ''}`} 
            onClick={(e) => { e.stopPropagation(); setSelectedIcon(app.id); setContextMenu(null); setStartOpen(false); }}
            onDoubleClick={() => { setSelectedIcon(null); openApp(app); }}
          >
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
          <div className="start-columns">
            <div className="start-apps" style={{overflowY: 'auto', height: '100%'}}>
              {FOLDERS.map(folder => (
                <details key={folder.id} className="start-folder-details" style={{listStyle: 'none'}}>
                  <summary className="start-item" style={{display: 'flex', alignItems: 'center', gap: '8px', cursor: 'pointer', listStyle: 'none'}}>
                    <img src={folder.icon} alt={folder.title} style={{width:'24px', height:'24px', imageRendering: 'pixelated'}} />
                    {folder.title} <span style={{marginLeft:'auto', fontSize:'10px', color:'rgba(255,255,255,0.5)'}}>▼</span>
                  </summary>
                  <div className="folder-contents" style={{paddingLeft: '16px', background: 'rgba(0,0,0,0.1)'}}>
                    {APPS.filter(a => a.folder === folder.id).map(app => (
                      <div key={app.id} className="start-item" onClick={() => openApp(app)} style={{display: 'flex', alignItems: 'center', gap: '8px'}}>
                        <img src={app.icon} alt={app.title} style={{width:'24px', height:'24px', imageRendering: 'pixelated'}} />
                        <span>{app.title}</span>
                      </div>
                    ))}
                  </div>
                </details>
              ))}
              {APPS.filter(a => !a.folder).map(app => (
                <div key={app.id} className="start-item" onClick={() => openApp(app)} style={{display: 'flex', alignItems: 'center', gap: '8px'}}>
                  <img src={app.icon} alt={app.title} style={{width:'24px', height:'24px', imageRendering: 'pixelated'}} />
                  <span>{app.title}</span>
                </div>
              ))}
            </div>
            <div className="start-system">
              <div className="start-user">
                <div className="start-avatar">
                  <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"></path><circle cx="12" cy="7" r="4"></circle></svg>
                </div>
                <span>KiloOS User</span>
              </div>
              <div className="start-system-actions">
                <div className="start-item" onClick={() => { setStartOpen(false); alert(`KiloOS Version ${MICROS_VERSION}`); }}>
                  About KiloOS
                </div>
                <a href="/exe/KApps.zip" download className="start-item" style={{textDecoration:'none', color:'inherit'}}>
                  Download .exe Pack
                </a>
                <div className="start-item" onClick={() => { setStartOpen(false); setModal({ type: 'shutdown' }); setModalInput('micros_state.json'); }}>
                  Shutdown...
                </div>
              </div>
            </div>
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
              {modal.type === 'shutdown' ? 'Shutdown KiloOS' : modal.type === 'vfs_open' ? 'Open Virtual File' : 'Save Virtual File'}
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
      
      {contextMenu && (
        <div 
          className="context-menu" 
          style={{left: contextMenu.x, top: contextMenu.y}}
        >
          <div className="context-item" onClick={() => { setContextMenu(null); window.location.reload(); }}>Refresh Desktop</div>
          <div className="context-item" onClick={() => { setContextMenu(null); alert('Wallpaper settings not available.'); }}>Change Wallpaper</div>
          <div className="context-separator" />
          <div className="context-item" onClick={() => { setContextMenu(null); alert('Display settings not available.'); }}>Display Settings</div>
          <div className="context-separator" style={{opacity: 0.1}} />
          <div className="context-item" style={{color: 'rgba(255,0,0,0.1)', cursor: 'crosshair'}} onClick={() => { setContextMenu(null); setSafeMode(!safeMode); }}>{safeMode ? 'Disable' : 'Enable'} Safe Mode</div>
        </div>
      )}
    </div>
  );
}

export default App;
