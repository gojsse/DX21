import { useEffect } from 'react'
import type { CSSProperties } from 'react'
import { useStore } from './state/store'
import { THEMES, THEME_NAMES } from './theme/themes'
import { NAV, TITLES } from './state/seed'
import type { ViewId } from './state/types'
import { useAudio } from './audio/useAudio'
import { VoiceView } from './views/VoiceView'
import { LibraryView } from './views/LibraryView'
import { DrumsView } from './views/DrumsView'
import { FxView } from './views/FxView'
import { GeneratorView } from './views/GeneratorView'
import { MidiView } from './views/MidiView'

export default function App() {
  const view = useStore((s) => s.view)
  const theme = useStore((s) => s.theme)
  const mode = useStore((s) => s.mode)
  const hardwareLink = useStore((s) => s.hardwareLink)
  const setView = useStore((s) => s.setView)
  const setTheme = useStore((s) => s.setTheme)
  const toggleHardwareLink = useStore((s) => s.toggleHardwareLink)
  const undo = useStore((s) => s.undo)

  // Undo everywhere (Cmd/Ctrl+Z).
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if ((e.metaKey || e.ctrlKey) && e.key.toLowerCase() === 'z' && !e.shiftKey) {
        e.preventDefault()
        undo()
      }
    }
    window.addEventListener('keydown', onKey)
    return () => window.removeEventListener('keydown', onKey)
  }, [undo])

  const cycleTheme = () => {
    const i = THEME_NAMES.indexOf(theme)
    setTheme(THEME_NAMES[(i + 1) % THEME_NAMES.length])
  }

  const { running: audioOn } = useAudio()

  // Active theme tokens applied as CSS custom properties on the chassis root —
  // switching `theme` restyles every view at once (mirrors the prototype rootVars).
  const rootVars: CSSProperties = {
    ...(THEMES[theme] as CSSProperties),
    width: 1200, maxWidth: '100%', background: 'var(--chassis)',
    border: '1px solid var(--chassis-b)', borderRadius: 10, boxShadow: '0 2px 16px rgba(0,0,0,.5)',
  }

  const linkDot: CSSProperties = {
    width: 7, height: 7, borderRadius: '50%',
    background: hardwareLink ? 'var(--acc)' : 'var(--faint)',
    boxShadow: hardwareLink ? '0 0 8px var(--accg)' : 'none',
    animation: hardwareLink ? 'blip 2.4s ease-in-out infinite' : 'none',
  }

  return (
    <div style={{ display: 'flex', justifyContent: 'center', padding: '28px 20px' }}>
      <div style={rootVars}>
        {/* header + nav */}
        <div style={{ display: 'flex', alignItems: 'center', gap: 16, padding: '11px 16px', background: 'var(--head)', borderBottom: '2px solid var(--head-b)', borderRadius: '10px 10px 0 0' }}>
          <div style={{ display: 'flex', flexDirection: 'column', gap: 3 }}>
            <div style={{ display: 'flex', alignItems: 'baseline', gap: 9 }}>
              <div style={{ font: "700 20px 'Barlow Condensed'", letterSpacing: '.05em', color: 'var(--ink)' }}>OP4</div>
              <div style={{ font: "600 9px 'Barlow Condensed'", letterSpacing: '.2em', color: 'var(--silk)' }}>{TITLES[view]}</div>
            </div>
            <div style={{ height: 2, width: 120, background: 'linear-gradient(90deg,var(--acc),var(--acc) 60%,transparent)' }} />
          </div>
          <div style={{ display: 'flex', gap: 3, marginLeft: 8 }}>
            {NAV.map(([id, label]) => {
              const active = view === id
              const st: CSSProperties = {
                padding: '5px 12px', borderRadius: 3, cursor: 'pointer',
                font: "600 10.5px 'Barlow Condensed'", letterSpacing: '.12em',
                color: active ? 'var(--keyA-ink)' : 'var(--nav-ink)',
                background: active ? 'var(--keyA)' : 'var(--nav)',
                border: '1px solid ' + (active ? 'var(--keyA-b)' : 'var(--nav-b)'),
                boxShadow: '0 2px 3px rgba(0,0,0,.3)', opacity: active ? 1 : 0.8,
              }
              return <div key={id} style={st} onClick={() => setView(id as ViewId)}>{label}</div>
            })}
          </div>
          <div style={{ flex: 1 }} />
          <div onClick={toggleHardwareLink} title="Toggle hardware link" style={{ display: 'flex', alignItems: 'center', gap: 8, cursor: 'pointer' }}>
            <div style={linkDot} />
            <div style={{ font: "600 10px 'Barlow Condensed'", letterSpacing: '.14em', color: 'var(--silk)' }}>
              {hardwareLink ? `LINK · ${mode} #1` : 'NO LINK'}
            </div>
          </div>
        </div>

        {/* active view */}
        {view === 'voice' && <VoiceView />}
        {view === 'lib' && <LibraryView />}
        {view === 'drums' && <DrumsView />}
        {view === 'fx' && <FxView />}
        {view === 'gen' && <GeneratorView />}
        {view === 'midi' && <MidiView />}

        {/* footer */}
        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', padding: '6px 16px 8px', borderTop: '1px solid var(--head-b)' }}>
          <div style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)' }}>ENGINE VINTAGE 8-VOICE · OVERSAMPLE 2×</div>
          <div style={{ display: 'flex', alignItems: 'center', gap: 6 }} title="Play with the A–K keys · Z / X shift octave">
            <div style={{ width: 6, height: 6, borderRadius: '50%', background: audioOn ? 'var(--acc)' : 'var(--faint)', boxShadow: audioOn ? '0 0 6px var(--accg)' : 'none' }} />
            <div style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)' }}>♪ A–K PLAY · Z/X OCTAVE</div>
          </div>
          <div onClick={cycleTheme} title="Click to cycle theme" style={{ font: "400 9.5px 'IBM Plex Mono'", color: 'var(--faint)', cursor: 'pointer' }}>OP4 1.2.0 · THEME {theme} ▸</div>
        </div>
      </div>
    </div>
  )
}
